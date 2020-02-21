
#include "shaders/app_config.cuh"

#include "include/Application.h"

// utilities library from nvidia SDK
#include <sutil.h>

// Devil
#include <IL/il.h>

#include <iostream>
#include <iomanip>
#include <sstream>

// Parse command line arguments and options
#include "include/Options.h"

static Application* g_app = nullptr;

//------------------------------------------------------------------------------
//
//  GLFW callbacks
//
//------------------------------------------------------------------------------
static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

// imgui will chain this in before it handles
void keyCallback( GLFWwindow* window, int key, int scancode, int action, int mods )
{
  static unsigned int saveCount = 0;

  if( action == GLFW_PRESS )
  {
    switch( key )
    {

    case GLFW_KEY_Q:  // Set state to exit app
    {
      glfwSetWindowShouldClose(window, 1);
      break;
    }

    case( GLFW_KEY_S ): // save screen shot, incrementing filename each time
    {
      std::stringstream filename;
      filename << "screenshot_"
               << std::setw(2) << std::setfill('0') << ++saveCount
               << ".png";
      const std::string outputImage = filename.str();
      std::cerr << "Saving current frame to '" << outputImage << "'\n";
      g_app->screenshot(outputImage);
      break;
    }
    } // end switch( key )
  }

}

int runApp(Options const& options)
{

  int widthClient  = std::max(1, options.getClientWidth());
  int heightClient = std::max(1, options.getClientHeight());

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
  GLFWwindow *window = glfwCreateWindow(widthClient, heightClient, "OptiX 6.5 Rayocaster", NULL, NULL);
  if (window == NULL) {
    glfw_error_callback(APP_ERROR_CREATE_WINDOW, "Failed to create GLFW window..");
    glfwTerminate();
    return APP_ERROR_CREATE_WINDOW;
  }

  // Set current OpenGL context
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
    glfw_error_callback(APP_ERROR_GLEW_INIT, "Failed to initialize OpenGL loader!");
    glfwTerminate();
    return  APP_ERROR_GLEW_INIT ;
  }
  else
  {
    std::cerr << "INFO: OpenGL renderer: "
              << glGetString(GL_RENDERER)
              << std::endl;

    std::cerr << "INFO: OpenGL version in GLFW window context: "
              << glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR)
              << "."
              << glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR)
              << std::endl;

    std::cerr << "INFO: GLFW version: "
              << glfwGetVersionString()
              << std::endl;
  }

  // Note: imgui now saves and chains any glfw callbacks registered
  glfwSetKeyCallback( window, keyCallback );

  ilInit(); // Initialize DevIL once.

  // start our Application context
  g_app = new Application(window, options);

  if (!g_app->isValid())
  {
    std::cerr << "ERROR: Application initialization failed." << std::endl;
    ilShutDown();
    glfwTerminate();
    return ( APP_ERROR_APP_INIT );
  }

  // Set initial OpenGL viewport
  glfwGetFramebufferSize(window, &widthClient, &heightClient);
  glViewport(0, 0, widthClient, heightClient);

  // main loop
  while (!glfwWindowShouldClose(window))
  {
    // Poll and handle events (inputs, window resize, etc.)
    glfwPollEvents();

    glfwGetFramebufferSize(window, &widthClient, &heightClient);

    g_app->reshape(widthClient, heightClient);

    if (options.hasGUI())
    {
      g_app->guiNewFrame();

      //g_app->guiDemoWindow(); // ImGui example code.

      g_app->guiWindow(); // The OptiX introduction example GUI window.

      g_app->guiEventHandler(); // Currently only reacting on SPACE to toggle the GUI window.

      g_app->render();  // OptiX rendering and OpenGL texture update.
      g_app->display(); // OpenGL display.

      g_app->guiRender(); // Render all ImGUI elements at last.

      glfwSwapBuffers(window);
    }
    else
    {
      std::string filenameScreenshot = options.getFilenameScreenshot();
      int numSamples = options.getNumberSamples();
      std::cerr << "Collecting " << numSamples << " samples per pixel..." << std::endl;
      for (int i = 0; i < numSamples; ++i) // Accumulate samples per pixel.
      {
        g_app->render();  // OptiX rendering and OpenGL texture update.
      }
      g_app->screenshot(filenameScreenshot);

      glfwSetWindowShouldClose(window, 1);
    }
  }

  // Cleanup
  delete g_app;

  ilShutDown();

  glfwDestroyWindow(window);

  glfwTerminate();

  return APP_EXIT_SUCCESS; // Success.
}


//------------------------------------------------------------------------------
//
//  main
//
//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  glfwSetErrorCallback(glfw_error_callback);

  if (!glfwInit())
  {
    glfw_error_callback(APP_ERROR_GLFW_INIT, "GLFW failed to initialize.");
    return APP_ERROR_GLFW_INIT;
  }

  int result = APP_ERROR_UNKNOWN;

  Options options;

  if (options.parseCommandLine(argc, argv))
  {
    result = runApp(options);
  }

  return result; // exit
}
