
#include "shaders/app_config.cuh"

#include "include/Application.h"

#include <sutil.h>

#include <IL/il.h>

#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <chrono>

// Parse command line arguments and options
#include "include/InputParser.h"


// #define Nx_MIN  (320)
// #define Ny_MIN  (200)

// // set maximum resolution ~ standard 4K dimensions
// #define Nx_MAX  (3840)
// #define Ny_MAX  (2240)  // was 2160, but increased so that square resolutions could hit 2240

// #define Nx_DEFAULT  (1200)
// #define Ny_DEFAULT  (600)

// #define Nscene_DEFAULT (0)
// #define Nscene_MAX  (4)   // Range [0 .. Nscene_MAX]

#define Nsamples_DEFAULT  (64)
#define Nsamples_MAX      (1024)

// Assign default GUI window size
#define GUI_WINDOW_DEFAULT_STARTING_Nx  (1280)
#define GUI_WINDOW_DEFAULT_STARTING_Ny  ( 720)

#define NOptix_Stack_Size_DEFAULT       (1024)
#define NMiss_Shader_DEFAULT            (2)
#define NMiss_Shader_MAX                (2)    // Range [0 .. NMiss_Shader_MAX]

static Application* g_app = nullptr;

// static bool displayGUI = true; // unimplemented currently

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void printUsage(const std::string& argv0)
{
  std::cerr << std::endl << "Usage: " << argv0 << " [options]" << std::endl;
  std::cerr << R"(
App Options:
  -h | --help | help   Print this usage message and exit."
  -v | --verbose       Verbose output.
  -g | --debug         Debug output.

)";
  std::cerr <<
    // "  -s | --scene <int>   Scene selection number, N: 0, 1, (max: " << Nscene_MAX << ')' << std::endl <<
    // "  -x | -dx <int>       Output image width, x dimension (default: "  << Nx_DEFAULT << ')' << std::endl <<
    // "  -y | -dy <int>       Output image height, y dimension (default: " << Ny_DEFAULT << ')' << std::endl <<
    // std::endl <<
    "  -w | --width <int>   GUI Window client width.  (default: "     << GUI_WINDOW_DEFAULT_STARTING_Nx << ')' << std::endl <<
    "  -e | --height <int>  GUI Window client height. (default: "     << GUI_WINDOW_DEFAULT_STARTING_Ny << ')' << std::endl <<
    "  -d | --devices <int> OptiX device selection, each decimal digit selects one device (default: 3210)."    << std::endl <<
    "  -n | --nopbo         Disable OpenGL interop for the image display. "    << std::endl <<
    "  -l | --light         Add an area light to the scene. "                  << std::endl <<
    "  -m | --miss  <0|1|2> Select the miss shader. (0 = black, 1 = white, 2 = HDR texture) (default: " << NMiss_Shader_DEFAULT << ')' << std::endl <<
    "    -i | --env <filename> Filename of a spherical HDR texture. Use with --miss 2. (default: "   << "???" << ')' << std::endl <<
    "  -k | --stack <int>   Set the OptiX stack size (1024) (debug feature). (default: " << NOptix_Stack_Size_DEFAULT << ')' << std::endl <<
    ""  << std::endl <<
    "  -f | --file <filename> Save image to file and exit."  << std::endl <<
    "    -p | --samples <int>       When saving to file, sample each pixel N times. (default: " << Nsamples_DEFAULT << ", max: " << Nsamples_MAX << ')' << std::endl <<
    "";


  std::cerr << R"(
App Keystrokes:
  SPACE  Toggles ImGui display.

)";

}


int main(int argc, char* argv[])
{
  int exit_code = EXIT_SUCCESS;

  // default values
  // int Nx = Nx_DEFAULT;
  // int Ny = Ny_DEFAULT;
  // int Nscene = Nscene_DEFAULT;
  int Nsamples = Nsamples_DEFAULT;
  bool Qverbose;
  bool Qdebug;

  int windowWidth = GUI_WINDOW_DEFAULT_STARTING_Nx;
  int windowHeight = GUI_WINDOW_DEFAULT_STARTING_Ny;
  int  devices      = 3210;  // Decimal digits encode OptiX device ordinals. Default 3210 means to use all four first installed devices, when available.
  bool interop      = true;  // Use OpenGL interop Pixel-Bufferobject to display the resulting image. Disable this when running on multi-GPU or TCC driver mode.
  bool light        = false; // Add a geometric are light. Best used with miss 0 and 1.
  int  miss         = NMiss_Shader_DEFAULT; // Select the environment light (0 = black, no light; 1 = constant white environment; 2 = spherical environment texture.
  int  stackSize    = NOptix_Stack_Size_DEFAULT ;  // Command line parameter just to be able to find the smallest working size.
  std::string environment = std::string(sutil::samplesDir()) + "/data/NV_Default_HDR_3000x1500.hdr";

  std::string filenameScreenshot;
  bool hasGUI = true;

  std::vector <std::string> sameOptionList;
  InputParser cl_input(argc, argv);

  sameOptionList.clear();
  sameOptionList.push_back("-h"); sameOptionList.push_back("--help");
  sameOptionList.push_back("?");  sameOptionList.push_back("help");
  if (cl_input.cmdEquivalentsExist(sameOptionList) ) {
    printUsage(argv[0]);
    std::exit( exit_code );
  }

  sameOptionList.clear();
  sameOptionList.push_back("-v"); sameOptionList.push_back("--verbose");
  Qverbose = cl_input.cmdEquivalentsExist(sameOptionList);

  sameOptionList.clear();
  sameOptionList.push_back("-g"); sameOptionList.push_back("--debug");
  Qdebug = cl_input.cmdEquivalentsExist(sameOptionList);

  sameOptionList.clear();
  sameOptionList.push_back("-n"); sameOptionList.push_back("--nopbo");
  if (cl_input.cmdEquivalentsExist(sameOptionList)) {
    interop = false;
  }

  sameOptionList.clear();
  sameOptionList.push_back("-l"); sameOptionList.push_back("--light");
  if (cl_input.cmdEquivalentsExist(sameOptionList)) {
    light = true;
  }

  // sameOptionList.clear();
  // sameOptionList.push_back("-s");  sameOptionList.push_back("--scene");
  // const std::string &sceneNumber = cl_input.getCmdEquivalentsOption(sameOptionList);
  // try {
  //   if (!sceneNumber.empty()){
  //     std::size_t pos;
  //     int x = std::stoi(sceneNumber, &pos);
  //     if (x >= 0 and x <= Nscene_MAX) {
  //       Nscene = x;
  //     } else {
  //       std::cerr << "WARNING: Scene number " << x << " out of range. Maximum scene number: " << Nscene_MAX << std::endl;
  //       std::cerr << "WARNING: Using a scene value of " << Nscene << std::endl;
  //     }
  //   }
  // } catch (std::invalid_argument const &ex) {
  //   printUsage(argv[0]);
  //   std::cerr << "ERROR: Invalid scene number: " << sceneNumber << std::endl;
  //   std::exit ( EXIT_FAILURE );
  // }

  sameOptionList.clear();
  sameOptionList.push_back("-p");  sameOptionList.push_back("--samples");
  const std::string &numberOfSamples = cl_input.getCmdEquivalentsOption(sameOptionList);
  try {
    if (!numberOfSamples.empty()){
      std::size_t pos;
      int x = std::stoi(numberOfSamples, &pos);
      if ( (x > 0) && (x <= Nsamples_MAX))  {
        Nsamples = x;
      } else {
        std::cerr << "WARNING: Number of samples " << x << " is out of range. ";
        if (x > Nsamples_MAX) {
          Nsamples = Nsamples_MAX;
        }
        std::cerr << "WARNING: Using a value of " << Nsamples << std::endl;
      }
    }
  } catch (std::invalid_argument const &ex) {
    printUsage(argv[0]);
    std::cerr << "ERROR: Invalid number of samples: " << numberOfSamples << std::endl;
    std::exit ( EXIT_FAILURE );
  }

  // sameOptionList.clear();
  // sameOptionList.push_back("-x");  sameOptionList.push_back("-dx");
  // const std::string &dimWidth =  cl_input.getCmdEquivalentsOption(sameOptionList);
  // try {
  //   if (!dimWidth.empty()){
  //     std::size_t pos;
  //     int x = std::stoi(dimWidth, &pos);
  //     if (x >= Nx_MIN and x <= Nx_MAX) {
  //       Nx = x;
  //     } else {
  //       std::cerr << "WARNING: Width (-dx) " << x << " out of range. ";
  //       if (x > Nx_MAX) {
  //         Nx = Nx_MAX;
  //       }
  //       if (x < Nx_MIN) {
  //         Nx = Nx_MIN;
  //       }
  //       std::cerr << "WARNING: Using a value of " << Nx <<std::endl;
  //     }
  //   }
  // } catch (std::invalid_argument const &ex) {
  //   printUsage(argv[0]);
  //   std::cerr << "ERROR: Invalid image width (-dx): " << dimWidth << std::endl;
  //   std::exit ( EXIT_FAILURE );
  // }

  // sameOptionList.clear();
  // sameOptionList.push_back("-y");  sameOptionList.push_back("-dy");
  // const std::string &dimHeight = cl_input.getCmdEquivalentsOption(sameOptionList);
  // try {
  //   if (!dimHeight.empty())
  //   {
  //     std::size_t pos;
  //     int x = std::stoi(dimHeight, &pos);
  //     if (x >= Ny_MIN and x <= Ny_MAX)
  //     {
  //       Ny = x;
  //     }
  //     else
  //     {
  //       std::cerr << "WARNING: Width (-dy) " << x << " out of range. ";
  //       if (x > Ny_MAX) {
  //         Ny = Ny_MAX;
  //       }
  //       if (x < Ny_MIN) {
  //         Ny = Ny_MIN;
  //       }
  //       std::cerr << "WARNING: Using a value of " << Ny <<std::endl;
  //     }
  //   }
  // } catch (std::invalid_argument const &ex) {
  //   printUsage(argv[0]);
  //   std::cerr << "ERROR: Invalid image height (-dy): " << dimHeight << std::endl;
  //   std::exit ( EXIT_FAILURE );
  // }

  sameOptionList.clear();
  sameOptionList.push_back("-w");  sameOptionList.push_back("--width");
  const std::string &winWidth =  cl_input.getCmdEquivalentsOption(sameOptionList);
  try {
    if (!winWidth.empty()){
      std::size_t pos;
      int x = std::stoi(winWidth, &pos);
      windowWidth = x;
    }
  } catch (std::invalid_argument const &ex) {
    printUsage(argv[0]);
    std::cerr << "ERROR: Invalid window width (--width): " << winWidth << std::endl;
    std::exit ( EXIT_FAILURE );
  }

  sameOptionList.clear();
  sameOptionList.push_back("-e");  sameOptionList.push_back("--height");
  const std::string &winHeight =  cl_input.getCmdEquivalentsOption(sameOptionList);
  try {
    if (!winHeight.empty()){
      std::size_t pos;
      int x = std::stoi(winHeight, &pos);
      windowHeight = x;
    }
  } catch (std::invalid_argument const &ex) {
    printUsage(argv[0]);
    std::cerr << "ERROR: Invalid window width (--height): " << winHeight << std::endl;
    std::exit ( EXIT_FAILURE );
  }

  sameOptionList.clear();
  sameOptionList.push_back("-d");  sameOptionList.push_back("--device");
  const std::string &deviceString =  cl_input.getCmdEquivalentsOption(sameOptionList);
  try {
    if (!deviceString.empty()){
      std::size_t pos;
      int x = std::stoi(deviceString, &pos);
      devices = x;
    }
  } catch (std::invalid_argument const &ex) {
    printUsage(argv[0]);
    std::cerr << "ERROR: Invalid stack size (--stack): " << deviceString << std::endl;
    std::exit ( EXIT_FAILURE );
  }


  sameOptionList.clear();
  sameOptionList.push_back("-k");  sameOptionList.push_back("--stack");
  const std::string &stackSizeString =  cl_input.getCmdEquivalentsOption(sameOptionList);
  try {
    if (!stackSizeString.empty()){
      std::size_t pos;
      int x = std::stoi(stackSizeString, &pos);
      stackSize = x;
    }
  } catch (std::invalid_argument const &ex) {
    printUsage(argv[0]);
    std::cerr << "ERROR: Invalid stack size (--stack): " << stackSizeString << std::endl;
    std::exit ( EXIT_FAILURE );
  }

  sameOptionList.clear();
  sameOptionList.push_back("-m");  sameOptionList.push_back("--miss");
  const std::string &missShader = cl_input.getCmdEquivalentsOption(sameOptionList);
  try {
    if (!missShader.empty()){
      std::size_t pos;
      int x = std::stoi(missShader, &pos);
      if (x >= 0 and x <= NMiss_Shader_MAX) {
        miss = x;
      } else {
        std::cerr << "WARNING: Miss shader number " << x << " out of range. Maximum: " << NMiss_Shader_MAX << std::endl;
        std::cerr << "WARNING: Using miss shader " << miss << std::endl;
      }
    }
  } catch (std::invalid_argument const &ex) {
    printUsage(argv[0]);
    std::cerr << "ERROR: Invalid miss shader number: " << missShader << std::endl;
    std::exit ( EXIT_FAILURE );
  }

  sameOptionList.clear();
  sameOptionList.push_back("-i");  sameOptionList.push_back("--env");
  const std::string &envImageFile =  cl_input.getCmdEquivalentsOption(sameOptionList);
  try {
    if (!envImageFile.empty()) {
      environment = envImageFile;
    }
  } catch (std::invalid_argument const &ex) {
    printUsage(argv[0]);
    std::cerr << "ERROR: Invalid filename (--env): " << envImageFile << std::endl;
    std::exit ( EXIT_FAILURE );
  }


  sameOptionList.clear();
  sameOptionList.push_back("-f");  sameOptionList.push_back("--file");
  const std::string &outputImageFile =  cl_input.getCmdEquivalentsOption(sameOptionList);
  try {
    if (!outputImageFile.empty()) {
      filenameScreenshot = outputImageFile;
      hasGUI = false;
    }
  } catch (std::invalid_argument const &ex) {
    printUsage(argv[0]);
    std::cerr << "ERROR: Invalid filename (--file): " << outputImageFile << std::endl;
    std::exit ( EXIT_FAILURE );
  }



  // Setup window after handling command line options
  glfwSetErrorCallback(glfw_error_callback);

  if (!glfwInit())
  {
    glfw_error_callback(1, "GLFW failed to initialize.");
    std::exit ( 1 ) ;
  }



// Decide GL version (set GL Hints before glfwCreateWindow)
// glxinfo -B
#if __APPLE__
// GL 3.2
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);		   // Required on Mac
  glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE); //
#else
// GL 3.0
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
#endif

// Create window with graphics context
  GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight, "OptiX 6.5 Rayocaster", NULL, NULL);
  if (window == NULL) {
    glfw_error_callback(2, "Failed to create GLFW window..");
    glfwTerminate();
    std::exit ( 2 );
  }

  // current OpenGL context
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

// Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
  bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
  bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
  bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING)
  bool err = false;
  glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
  bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif

  if (err)
  {
    glfw_error_callback(3, "Failed to initialize OpenGL loader!");
    glfwTerminate();
    std::exit ( 3 );
  }
  else
  {

    std::cerr << "[INFO] OpenGL renderer: "
              << glGetString(GL_RENDERER)
              << std::endl;

    // std::cerr << "[INFO] OpenGL "
    //           << GLVersion.major << "." << GLVersion.minor
    //           << std::endl;

    std::cerr << "[INFO] OpenGL from GLFW "
              << glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR)
              << "."
              << glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR)
              << std::endl;

    std::cerr << "[INFO] GLFW version: "
              << glfwGetVersionString()
              << std::endl;
  }

  // set initial OpenGL viewport
  glfwGetFramebufferSize(window, &windowWidth, &windowWidth);
  glViewport(0, 0, windowWidth, windowHeight);

  ilInit(); // Initialize DevIL once.

  // start our Application context
  g_app = new Application(window, windowWidth, windowHeight,
                          devices, stackSize, interop, light, miss, environment);

  if (!g_app->isValid())
  {
    glfw_error_callback(4, "Application initialization failed.");
    ilShutDown();
    glfwTerminate();
    std::exit ( 4 );
  }

  // main loop
  while (!glfwWindowShouldClose(window))
  {
    // Poll and handle events (inputs, window resize, etc.)
    glfwPollEvents(); // Render continuously.

    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    g_app->reshape(windowWidth, windowHeight);

    if (hasGUI)
    {

      g_app->guiNewFrame();

      //g_app->guiDemoWindow(); // ImGui example code.

      g_app->guiWindow(); // The OptiX introduction example GUI window.

      g_app->guiEventHandler(); // Currently only reacting on SPACE to toggle the GUI window.

      g_app->render();  // OptiX rendering and OpenGL texture update.
      g_app->display(); // OpenGL display. // ERROR 1282 in glEnd


      g_app->guiRender(); // Render all ImGUI elements at last.

      glfwSwapBuffers(window);

    }

    else
    {
      for (int i = 0; i < Nsamples; ++i) // Accumulate samples per pixel.
      {
        g_app->render();  // OptiX rendering and OpenGL texture update.
      }
      g_app->screenshot(filenameScreenshot);

      glfwSetWindowShouldClose(window, 1);
    }

  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);

  ilShutDown();

  //delete g_app;

  glfwTerminate();

  std::exit ( EXIT_SUCCESS);
}
