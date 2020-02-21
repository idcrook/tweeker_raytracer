
#include <sutil.h>

#include <iostream>
#include <iomanip>
#include <stdexcept>

// Parse command line arguments and options
#include "include/Options.h"
#include "include/InputParser.h"

#define NSamples_DEFAULT  (64)
#define NSamples_MAX      (1024)

// Assign default GUI window size
#define GUI_WINDOW_DEFAULT_STARTING_Nx  (1280)
#define GUI_WINDOW_DEFAULT_STARTING_Ny  ( 720)

#define NOptix_Stack_Size_DEFAULT       (1024)
#define NMiss_Shader_DEFAULT            (2)
#define NMiss_Shader_MAX                (2)    // Range [0 .. NMiss_Shader_MAX]


Options::Options()
  : m_verbose(0)
  , m_debug(false)
  , m_widthClient(GUI_WINDOW_DEFAULT_STARTING_Nx)
  , m_heightClient(GUI_WINDOW_DEFAULT_STARTING_Ny)
  , m_devices(3210)              // Decimal digits encode OptiX device ordinals. Default 3210 means to use all four first installed devices, when available.
  , m_interop(true)              // Use OpenGL interop Pixel-Bufferobject to display the resulting image. Disable this when running on multi-GPU or TCC driver mode.
  , m_light(0)                   // Add a geometric area light. Best used with miss 0 and 1.
  , m_miss(NMiss_Shader_DEFAULT) // Select the environment light (0 = black, no light; 1 = constant white environment; 2 = spherical environment texture.
  , m_numberSamples(NSamples_DEFAULT)  // Number of samples per pixel to collect when saving to file from command line.
  , m_stackSize(NOptix_Stack_Size_DEFAULT)  // Command line parameter just to be able to find the smallest working size.
  , m_environment(std::string(sutil::samplesDir()) + "/data/NV_Default_HDR_3000x1500.hdr")
//  , m_filenameScreenshot // is std::string
  , m_hasGUI(true)
{
}

Options::~Options()
{
}

bool Options::parseCommandLine(int argc, char *argv[])
{
  int exit_code = EXIT_SUCCESS;

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
  m_verbose = cl_input.cmdEquivalentsExist(sameOptionList);

  sameOptionList.clear();
  sameOptionList.push_back("-g"); sameOptionList.push_back("--debug");
  m_debug = cl_input.cmdEquivalentsExist(sameOptionList);

  sameOptionList.clear();
  sameOptionList.push_back("-n"); sameOptionList.push_back("--nopbo");
  if (cl_input.cmdEquivalentsExist(sameOptionList)) {
    m_interop = false;
  }

  sameOptionList.clear();
  sameOptionList.push_back("-l"); sameOptionList.push_back("--light");
  if (cl_input.cmdEquivalentsExist(sameOptionList)) {
    m_light = 1;
  }

  sameOptionList.clear();
  sameOptionList.push_back("-p");  sameOptionList.push_back("--samples");
  const std::string &numberOfSamples = cl_input.getCmdEquivalentsOption(sameOptionList);
  try {
    if (!numberOfSamples.empty()){
      std::size_t pos;
      int x = std::stoi(numberOfSamples, &pos);
      if ( (x > 0) && (x <= NSamples_MAX))  {
        m_numberSamples = x;
      } else {
        std::cerr << "WARNING: Number of samples " << x << " is out of range. ";
        if (x > NSamples_MAX) {
          m_numberSamples = NSamples_MAX;
        }
        std::cerr << "WARNING: Using a value of " << m_numberSamples << std::endl;
      }
    }
  } catch (std::invalid_argument const &ex) {
    printUsage(argv[0]);
    std::cerr << "ERROR: Invalid number of samples: " << numberOfSamples << std::endl;
    return false;
  }

  sameOptionList.clear();
  sameOptionList.push_back("-w");  sameOptionList.push_back("--width");
  const std::string &winWidth =  cl_input.getCmdEquivalentsOption(sameOptionList);
  try {
    if (!winWidth.empty()){
      std::size_t pos;
      int x = std::stoi(winWidth, &pos);
      m_widthClient = x;
    }
  } catch (std::invalid_argument const &ex) {
    printUsage(argv[0]);
    std::cerr << "ERROR: Invalid window width (--width): " << winWidth << std::endl;
    return false;
  }

  sameOptionList.clear();
  sameOptionList.push_back("-e");  sameOptionList.push_back("--height");
  const std::string &winHeight =  cl_input.getCmdEquivalentsOption(sameOptionList);
  try {
    if (!winHeight.empty()){
      std::size_t pos;
      int x = std::stoi(winHeight, &pos);
      m_heightClient = x;
    }
  } catch (std::invalid_argument const &ex) {
    printUsage(argv[0]);
    std::cerr << "ERROR: Invalid window width (--height): " << winHeight << std::endl;
    return false;
  }

  sameOptionList.clear();
  sameOptionList.push_back("-d");  sameOptionList.push_back("--device");
  const std::string &deviceString =  cl_input.getCmdEquivalentsOption(sameOptionList);
  try {
    if (!deviceString.empty()){
      std::size_t pos;
      int x = std::stoi(deviceString, &pos);
      m_devices = x;
    }
  } catch (std::invalid_argument const &ex) {
    printUsage(argv[0]);
    std::cerr << "ERROR: Invalid stack size (--stack): " << deviceString << std::endl;
    return false;
  }

  sameOptionList.clear();
  sameOptionList.push_back("-k");  sameOptionList.push_back("--stack");
  const std::string &stackSizeString =  cl_input.getCmdEquivalentsOption(sameOptionList);
  try {
    if (!stackSizeString.empty()){
      std::size_t pos;
      int x = std::stoi(stackSizeString, &pos);
      m_stackSize = x;
    }
  } catch (std::invalid_argument const &ex) {
    printUsage(argv[0]);
    std::cerr << "ERROR: Invalid stack size (--stack): " << stackSizeString << std::endl;
    return false;
  }

  sameOptionList.clear();
  sameOptionList.push_back("-m");  sameOptionList.push_back("--miss");
  const std::string &missShader = cl_input.getCmdEquivalentsOption(sameOptionList);
  try {
    if (!missShader.empty()){
      std::size_t pos;
      int x = std::stoi(missShader, &pos);
      if (x >= 0 and x <= NMiss_Shader_MAX) {
        m_miss = x;
      } else {
        std::cerr << "WARNING: Miss shader number " << x << " out of range. Maximum: " << NMiss_Shader_MAX << std::endl;
        std::cerr << "WARNING: Using miss shader " << m_miss << std::endl;
      }
    }
  } catch (std::invalid_argument const &ex) {
    printUsage(argv[0]);
    std::cerr << "ERROR: Invalid miss shader number: " << missShader << std::endl;
    return false;
  }

  sameOptionList.clear();
  sameOptionList.push_back("-i");  sameOptionList.push_back("--env");
  const std::string &envImageFile =  cl_input.getCmdEquivalentsOption(sameOptionList);
  try {
    if (!envImageFile.empty()) {
      m_environment = envImageFile;
    }
  } catch (std::invalid_argument const &ex) {
    printUsage(argv[0]);
    std::cerr << "ERROR: Invalid filename (--env): " << envImageFile << std::endl;
    return false;
  }

  sameOptionList.clear();
  sameOptionList.push_back("-f");  sameOptionList.push_back("--file");
  const std::string &outputImageFile =  cl_input.getCmdEquivalentsOption(sameOptionList);
  try {
    if (!outputImageFile.empty()) {
      m_filenameScreenshot = outputImageFile;
      m_hasGUI = false;
    }
  } catch (std::invalid_argument const &ex) {
    printUsage(argv[0]);
    std::cerr << "ERROR: Invalid filename (--file): " << outputImageFile << std::endl;
    return false;
  }

  return true;
}


int Options::getClientWidth() const
{
  return m_widthClient;
}

int Options::getClientHeight() const
{
  return m_heightClient;
}

unsigned int Options::getDevicesEncoding() const
{
  return m_devices;
}

unsigned int Options::getStackSize() const
{
  return m_stackSize;
}

bool Options::getInterop() const
{
  return m_interop;
}

int Options::getLight() const
{
  return m_light;
}

unsigned int Options::getMiss() const
{
  return m_miss;
}

int Options::getNumberSamples() const
{
  return m_numberSamples;
}

std::string Options::getEnvironment() const
{
  return m_environment;
}

std::string Options::getFilenameScreenshot() const
{
  return m_filenameScreenshot;
}

bool Options::hasGUI() const
{
  return m_hasGUI;
}

void Options::printUsage(const std::string& argv0)
{
  std::cerr << std::endl << "Usage: " << argv0 << " [options]" << std::endl;
  std::cerr << R"(
App Options:
  -h | --help | help   Print this usage message and exit."
  -v | --verbose       Verbose output. TBD
  -g | --debug         Debug output. TBD

)";
  std::cerr <<
    "  -w | --width <int>   GUI Window client width.  (default: "     << GUI_WINDOW_DEFAULT_STARTING_Nx << ')' << std::endl <<
    "  -e | --height <int>  GUI Window client height. (default: "     << GUI_WINDOW_DEFAULT_STARTING_Ny << ')' << std::endl <<
    "  -d | --devices <int> OptiX device selection, each decimal digit selects one device (default: 3210)."    << std::endl <<
    "  -n | --nopbo         Disable OpenGL interop for the image display. "    << std::endl <<
    "  -l | --light         Add an area light to the scene. "                  << std::endl <<
    "  -m | --miss  <0|1|2> Select the miss shader. (0 = black, 1 = white, 2 = HDR texture) (default: " << NMiss_Shader_DEFAULT << ')' << std::endl <<
    "    -i | --env <filename> Filename of a spherical HDR texture. Use with --miss 2. (default: "   << m_environment << ')' << std::endl <<
    "  -k | --stack <int>   Set the OptiX stack size (1024) (debug feature). (default: " << NOptix_Stack_Size_DEFAULT << ')' << std::endl <<
    ""  << std::endl <<
    "  -f | --file <filename> Save image to file and exit."  << std::endl <<
    "    -p | --samples <int>       When saving to file, sample each pixel N times. (default: " << NSamples_DEFAULT << ", max: " << NSamples_MAX << ')' << std::endl <<
    "";

  std::cerr << R"(
App Keystrokes:
  SPACE  Toggles ImGui display.
  s      Save a snapshot of current image to file.
  q      Quits the App.

)";

}
