/*
 * Copyright (c) 2013-2018, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Application.h"

#include <optix.h>
#include <optixu/optixpp.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include <optixu/optixu_quaternion_namespace.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>

// DAR Only for sutil::samplesPTXDir() and sutil::writeBufferToFile()
#include <sutil.h>

#include "MyAssert.h"

// DAR HACK Taken from per_ray_data.h. I don't need any of the rest of it in this source.
#define FLAG_THINWALLED 0x00000020

const char* const SAMPLE_NAME = "optixIntro_10";

// This only runs inside the OptiX Advanced Samples location,
// unless the environment variable OPTIX_SAMPLES_SDK_PTX_DIR is set.
// A standalone application which should run anywhere would place the *.ptx files
// into a subdirectory next to the executable and use a relative file path here!
static std::string ptxPath(std::string const& cuda_file)
{
  return std::string(sutil::samplesPTXDir()) + std::string("/") +
         std::string(SAMPLE_NAME) + std::string("_generated_") + cuda_file + std::string(".ptx");
}


Application::Application(GLFWwindow* window,
                         const int width,
                         const int height,
                         const unsigned int devices,
                         const unsigned int stackSize,
                         const bool interop,
                         const bool light,
                         const unsigned int miss,
                         std::string const& environment)
: m_window(window)
, m_width(width)
, m_height(height)
, m_devicesEncoding(devices)
, m_stackSize(stackSize)
, m_interop(interop)
, m_light(light)
, m_missID(miss)
, m_environmentFilename(environment)
{

// Decide GLSL versions
#if __APPLE__
// GLSL 150
  const char *glsl_version = "#version 150";
#else
// GLSL 130
  const char *glsl_version = "#version 130";
#endif

  // Setup ImGui binding.
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  ImGuiStyle& style = ImGui::GetStyle();

  // Style the GUI colors to a neutral greyscale with plenty of transaparency to concentrate on the image.
  // Change these RGB values to get any other tint.
  const float r = 1.0f;
  const float g = 1.0f;
  const float b = 1.0f;

  style.Colors[ImGuiCol_Text]                  = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
  style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
  style.Colors[ImGuiCol_WindowBg]              = ImVec4(r * 0.2f, g * 0.2f, b * 0.2f, 0.6f);
  style.Colors[ImGuiCol_ChildBg]               = ImVec4(r * 0.2f, g * 0.2f, b * 0.2f, 1.0f); // changed in v1.75
  style.Colors[ImGuiCol_PopupBg]               = ImVec4(r * 0.2f, g * 0.2f, b * 0.2f, 1.0f);
  style.Colors[ImGuiCol_Border]                = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.4f);
  style.Colors[ImGuiCol_BorderShadow]          = ImVec4(r * 0.0f, g * 0.0f, b * 0.0f, 0.4f);
  style.Colors[ImGuiCol_FrameBg]               = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.4f);
  style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.6f);
  style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_TitleBg]               = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.6f);
  style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(r * 0.2f, g * 0.2f, b * 0.2f, 0.2f);
  style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(r * 0.2f, g * 0.2f, b * 0.2f, 1.0f);
  style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(r * 0.2f, g * 0.2f, b * 0.2f, 0.2f);
  style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.4f);
  style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.6f);
  style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_CheckMark]             = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_SliderGrab]            = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.4f);
  style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_Button]                = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.4f);
  style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.6f);
  style.Colors[ImGuiCol_ButtonActive]          = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_Header]                = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.4f);
  style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.6f);
  style.Colors[ImGuiCol_HeaderActive]          = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  // removed in v1.72 // style.Colors[ImGuiCol_Column]                = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.4f);
  // removed in v1.72 // style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.6f);
  // removed in v1.72 // style.Colors[ImGuiCol_ColumnActive]          = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.6f);
  style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(r * 1.0f, g * 1.0f, b * 1.0f, 1.0f);
  // removed in v1.60 // style.Colors[ImGuiCol_CloseButton]           = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.4f);
  // removed in v1.60 // style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.6f);
  // removed in v1.60 // style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_PlotLines]             = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 1.0f);
  style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(r * 1.0f, g * 1.0f, b * 1.0f, 1.0f);
  style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 1.0f);
  style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(r * 1.0f, g * 1.0f, b * 1.0f, 1.0f);
  style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(r * 0.5f, g * 0.5f, b * 0.5f, 1.0f);
  style.Colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(r * 0.2f, g * 0.2f, b * 0.2f, 0.2f);  // renamed in v1.63
  style.Colors[ImGuiCol_DragDropTarget]        = ImVec4(r * 1.0f, g * 1.0f, b * 0.0f, 1.0f); // Yellow
  style.Colors[ImGuiCol_NavHighlight]          = ImVec4(r * 1.0f, g * 1.0f, b * 1.0f, 1.0f);
  style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(r * 1.0f, g * 1.0f, b * 1.0f, 1.0f);

  // Renderer setup and GUI parameters.
  m_minPathLength       = 2;    // Minimum path length after which Russian Roulette path termination starts.
  m_maxPathLength       = 6;    // Maximum path length. Need at least 6 segments to see a diffuse surface through a sphere.
  m_sceneEpsilonFactor  = 500;  // Factor on 1e-7 used to offset ray origins along the path to reduce self intersections.
  m_environmentRotation = 0.0f; // Not rotated, default camera setup looks down the negative z-axis which is the center of this image.

  m_present         = false;  // Update once per second. (The first half second shows all frames to get some initial accumulation).
  m_presentNext     = true;
  m_presentAtSecond = 1.0;

  m_builder = std::string("Trbvh");

  m_cameraType = LENS_SHADER_PINHOLE;

  m_shutterType = 0; // Stochastic.

  m_frames = 0; // Samples per pixel. 0 == render forever.

  // GLSL shaders objects and program.
  // In OptiX 5.1.0 the denoiser supports HDR beauty buffers which this example demonstrates.
  // Means the previous raygeneration entry point doing the tonemapping inside the CommandList can go away again
  // and the GLSL tonemapper can run afterwards as post-process as without the denoiser.
  m_glslVS      = 0;
  m_glslFS      = 0;
  m_glslProgram = 0;

#if 1 // Tonemapper defaults
  m_gamma          = 2.2f;
  m_colorBalance   = optix::make_float3(1.0f, 1.0f, 1.0f);
  m_whitePoint     = 1.0f;
  m_burnHighlights = 0.8f;
  m_crushBlacks    = 0.2f;
  m_saturation     = 1.2f;
  m_brightness     = 0.8f;
#else // DAR DEBUG Neutral tonemapper settings.
  m_gamma          = 1.0f;
  m_colorBalance   = optix::make_float3(1.0f, 1.0f, 1.0f);
  m_whitePoint     = 1.0f;
  m_burnHighlights = 1.0f;
  m_crushBlacks    = 0.0f;
  m_saturation     = 1.0f;
  m_brightness     = 1.0f;
#endif

  m_guiState = GUI_STATE_NONE;

  m_isWindowVisible = true;

  m_mouseSpeedRatio = 10.0f;

#if USE_DENOISER
  m_denoiseBlend = 0.0f; // 0.0f == denoised image, 1.0f == original image.
#endif

  m_pinholeCamera.setViewport(m_width, m_height);

  initOpenGL();
  initOptiX(); // Sets m_isValid when OptiX initialization was successful.
}

Application::~Application()
{
  // DAR FIXME Do any other destruction here.
  if (m_isValid)
  {
    m_context->destroy();
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  ImGui::DestroyContext();
}

bool Application::isValid() const
{
  return m_isValid;
}

void Application::reshape(int width, int height)
{
  if ((width != 0 && height != 0) && // Zero sized interop buffers are not allowed in OptiX.
      (m_width != width || m_height != height))
  {
    m_width  = width;
    m_height = height;

    glViewport(0, 0, m_width, m_height);

    restartAccumulation();

    try
    {
      m_bufferOutput->setSize(m_width, m_height); // RGBA32F buffer.

#if USE_DENOISER
      m_bufferDenoised->setSize(m_width, m_height); // RGBA32F buffer.
      if (m_interop)
      {
        m_bufferDenoised->unregisterGLBuffer(); // Must unregister or CUDA won't notice the size change and crash.
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_bufferDenoised->getGLBOId());
        glBufferData(GL_PIXEL_UNPACK_BUFFER, m_bufferDenoised->getElementSize() * m_width * m_height, nullptr, GL_STREAM_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        m_bufferDenoised->registerGLBuffer();
      }

#if USE_DENOISER_ALBEDO
      m_bufferAlbedo->setSize(m_width, m_height);     // RGBA32F buffer.
#if USE_DENOISER_NORMAL
      m_bufferNormals->setSize(m_width, m_height);    // RGBA32F buffer.
#endif
#endif

      // Because the CommandList has no interface to set launch dimensions per stage, build a new one with the new size.
      if (m_commandListDenoiser && m_stageDenoiser)
      {
        m_commandListDenoiser->destroy();

        m_commandListDenoiser = m_context->createCommandList();

        m_commandListDenoiser->appendPostprocessingStage(m_stageDenoiser, m_width, m_height);
        m_commandListDenoiser->finalize();
      }
#else
      // When not using the denoiser this is the buffer which is displayed.
      if (m_interop)
      {
        m_bufferOutput->unregisterGLBuffer(); // Must unregister or CUDA won't notice the size change and crash.
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_bufferOutput->getGLBOId());
        glBufferData(GL_PIXEL_UNPACK_BUFFER, m_bufferOutput->getElementSize() * m_width * m_height, nullptr, GL_STREAM_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        m_bufferOutput->registerGLBuffer();
      }
#endif
    }
    catch(optix::Exception& e)
    {
      std::cerr << e.getErrorString() << std::endl;
    }

    m_pinholeCamera.setViewport(m_width, m_height);

    restartAccumulation();
  }
}

void Application::guiNewFrame()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void Application::guiReferenceManual()
{
  // ImGui::ShowDemoWindow();
}

void Application::guiRender()
{
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::getSystemInformation()
{
  unsigned int optixVersion;
  RT_CHECK_ERROR_NO_CONTEXT(rtGetVersion(&optixVersion));

  unsigned int major = optixVersion / 1000; // Check major with old formula.
  unsigned int minor;
  unsigned int micro;
  if (3 < major) // New encoding since OptiX 4.0.0 to get two digits micro numbers?
  {
    major =  optixVersion / 10000;
    minor = (optixVersion % 10000) / 100;
    micro =  optixVersion % 100;
  }
  else // Old encoding with only one digit for the micro number.
  {
    minor = (optixVersion % 1000) / 10;
    micro =  optixVersion % 10;
  }
  std::cerr << "OptiX " << major << "." << minor << "." << micro << std::endl;

  unsigned int numberOfDevices = 0;
  RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetDeviceCount(&numberOfDevices));
  std::cerr << "Number of Devices = " << numberOfDevices << std::endl << std::endl;

  for (unsigned int i = 0; i < numberOfDevices; ++i)
  {
    char name[256];
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_NAME, sizeof(name), name));
    std::cerr << "Device " << i << ": " << name << std::endl;

    int computeCapability[2] = {0, 0};
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY, sizeof(computeCapability), &computeCapability));
    std::cerr << "  Compute Support: " << computeCapability[0] << "." << computeCapability[1] << std::endl;

    RTsize totalMemory = 0;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_TOTAL_MEMORY, sizeof(totalMemory), &totalMemory));
    std::cerr << "  Total Memory: " << (unsigned long long) totalMemory << std::endl;

    int clockRate = 0;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_CLOCK_RATE, sizeof(clockRate), &clockRate));
    std::cerr << "  Clock Rate: " << clockRate << " kHz" << std::endl;

    int maxThreadsPerBlock = 0;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK, sizeof(maxThreadsPerBlock), &maxThreadsPerBlock));
    std::cerr << "  Max. Threads per Block: " << maxThreadsPerBlock << std::endl;

    int smCount = 0;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, sizeof(smCount), &smCount));
    std::cerr << "  Streaming Multiprocessor Count: " << smCount << std::endl;

    int executionTimeoutEnabled = 0;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_EXECUTION_TIMEOUT_ENABLED, sizeof(executionTimeoutEnabled), &executionTimeoutEnabled));
    std::cerr << "  Execution Timeout Enabled: " << executionTimeoutEnabled << std::endl;

    int maxHardwareTextureCount = 0 ;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_MAX_HARDWARE_TEXTURE_COUNT, sizeof(maxHardwareTextureCount), &maxHardwareTextureCount));
    std::cerr << "  Max. Hardware Texture Count: " << maxHardwareTextureCount << std::endl;

    int tccDriver = 0;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_TCC_DRIVER, sizeof(tccDriver), &tccDriver));
    std::cerr << "  TCC Driver enabled: " << tccDriver << std::endl;

    int cudaDeviceOrdinal = 0;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_CUDA_DEVICE_ORDINAL, sizeof(cudaDeviceOrdinal), &cudaDeviceOrdinal));
    std::cerr << "  CUDA Device Ordinal: " << cudaDeviceOrdinal << std::endl << std::endl;
  }
}

void Application::initOpenGL()
{
  //glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
  glClear(GL_COLOR_BUFFER_BIT);

  glViewport(0, 0, m_width, m_height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (m_interop)
  {
    // PBO for the fast OptiX sysOutputBuffer to texture transfer.
    glGenBuffers(1, &m_pboOutputBuffer);
    MY_ASSERT(m_pboOutputBuffer != 0);
    // Buffer size must be > 0 or OptiX can't create a buffer from it.
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboOutputBuffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, m_width * m_height * sizeof(float) * 4, (void*) 0, GL_STREAM_READ); // RGBA32F from byte offset 0 in the pixel unpack buffer.
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  }

  // glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // default, works for BGRA8, RGBA16F, and RGBA32F.

  glGenTextures(1, &m_hdrTexture);
  MY_ASSERT(m_hdrTexture != 0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_hdrTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_2D, 0);

  // DAR ImGui has been changed to push the GL_TEXTURE_BIT so that this works.
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  initGLSL();
}


void Application::initOptiX()
{
  try
  {
    getSystemInformation();

    m_context = optix::Context::create();

    // Select the GPUs to use with this context.
    unsigned int numberOfDevices = 0;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetDeviceCount(&numberOfDevices));
    std::cerr << "Number of Devices = " << numberOfDevices << std::endl << std::endl;

    std::vector<int> devices;

    int devicesEncoding = m_devicesEncoding; // Preserve this information, it can be stored in the system file.
    unsigned int i = 0;
    do
    {
      int device = devicesEncoding % 10;
      devices.push_back(device); // DAR FIXME Should be a std::set to prevent duplicate device IDs in m_devicesEncoding.
      devicesEncoding /= 10;
      ++i;
    } while (i < numberOfDevices && devicesEncoding);

    m_context->setDevices(devices.begin(), devices.end());

    // Print out the current configuration to make sure what's currently running.
    devices = m_context->getEnabledDevices();
    for (size_t i = 0; i < devices.size(); ++i)
    {
      std::cerr << "m_context is using local device " << devices[i] << ": " << m_context->getDeviceName(devices[i]) << std::endl;
    }
    std::cerr << "OpenGL interop is " << ((m_interop) ? "enabled" : "disabled") << std::endl;

    initPrograms();
    initRenderer();
    //initScene();

    m_isValid = true; // If we get here with no exception, flag the initialization as successful. Otherwise the app will exit with error message.
  }
  catch(optix::Exception& e)
  {
    std::cerr << e.getErrorString() << std::endl;
  }
}

static void callbackUsageReport(int level, const char* tag, const char* msg, void* cbdata)
{
  std::cerr << "[" << level << "][" << std::left << std::setw(12) << tag << "] " << msg;
}


void Application::initRenderer()
{
  try
  {
    m_context->setEntryPointCount(1); // 0 = render // Tonemapper is a GLSL shader in this case.
    m_context->setRayTypeCount(2);    // 0 = radiance, 1 = shadow

    m_context->setStackSize(m_stackSize);
    std::cerr << "stackSize = " << m_stackSize << std::endl;

#if USE_DEBUG_EXCEPTIONS
    // Disable this by default for performance, otherwise the stitched PTX code will have lots of exception handling inside.
    m_context->setPrintEnabled(true);
    //m_context->setPrintLaunchIndex(256, 256);
    m_context->setExceptionEnabled(RT_EXCEPTION_ALL, true);
#endif

    // OptiX Usage Reports.
    // verbosity = 0: usage reports off
    // verbosity = 1: enables error messages and important warnings.
    // verbosity = 2: additionally enables minor warnings, performance recommendations, and scene statistics at startup or recompilation granularity.
    // verbosity = 3: additionally enables informational messages and per-launch statistics and messages.
    //m_context->setUsageReportCallback(callbackUsageReport, 3, NULL);

    // Add context-global variables here.
    m_context["sysSceneEpsilon"]->setFloat(m_sceneEpsilonFactor * 1e-7f);
    m_context["sysPathLengths"]->setInt(m_minPathLength, m_maxPathLength);
    m_context["sysEnvironmentRotation"]->setFloat(m_environmentRotation);
    m_context["sysIterationIndex"]->setInt(0); // With manual accumulation, 0 fills the buffer, accumulation starts at 1. On the VCA this variable is unused!

    // RT_BUFFER_INPUT_OUTPUT to support accumulation.
#if USE_DENOISER
    // Note that on multi-GPU systems it's not possible to use RT_BUFFER_GPU_LOCAL buffers directly as input into the denoiser,
    // because it wouldn't be able to combine the partial results from the different devices automatically.
    m_bufferOutput = m_context->createBuffer(RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT4, m_width, m_height); // RGBA32F, the noisy beauty image.
#else
    // In case of an OpenGL interop buffer, that is automatically registered with CUDA now! Must unregister/register around size changes.
    // The RT_BUFFER_GPU_LOCAL could be used on a separate accumulation buffer to improve performance on multi-GPU systems.
    m_bufferOutput = (m_interop) ? m_context->createBufferFromGLBO(RT_BUFFER_INPUT_OUTPUT, m_pboOutputBuffer)
                                 : m_context->createBuffer(RT_BUFFER_INPUT_OUTPUT);
    m_bufferOutput->setFormat(RT_FORMAT_FLOAT4); // RGBA32F
    m_bufferOutput->setSize(m_width, m_height);
#endif
    m_context["sysOutputBuffer"]->set(m_bufferOutput);

    /*  std::map<std::string, optix::Program>::const_iterator it = m_mapOfPrograms.find("raygeneration");
    MY_ASSERT(it != m_mapOfPrograms.end());
    m_context->setRayGenerationProgram(0, it->second); // entrypoint

    it = m_mapOfPrograms.find("exception");
    MY_ASSERT(it != m_mapOfPrograms.end());
    m_context->setExceptionProgram(0, it->second); // entrypoint

    it = m_mapOfPrograms.find("miss");
    MY_ASSERT(it != m_mapOfPrograms.end());
    m_context->setMissProgram(0, it->second); // raytype
*/
    // Default initialization. Will be overwritten on the first frame.
    m_context["sysCameraPosition"]->setFloat(0.0f, 0.0f, 1.0f);
    m_context["sysCameraU"]->setFloat(1.0f, 0.0f, 0.0f);
    m_context["sysCameraV"]->setFloat(0.0f, 1.0f, 0.0f);
    m_context["sysCameraW"]->setFloat(0.0f, 0.0f, -1.0f);

    // Lens shader selection.
    m_context["sysCameraType"]->setInt(m_cameraType);

    // Camera shutter selection
    m_context["sysShutterType"]->setInt(m_shutterType);

#if USE_DENOISER
    // Initialize the HDR denoiser.
    // The HDR float4 buffer after denoising. This one will be tonemapped and displayed with OpenGL when the denoiser is active.
    m_bufferDenoised = (m_interop) ? m_context->createBufferFromGLBO(RT_BUFFER_OUTPUT, m_pboOutputBuffer)
                                   : m_context->createBuffer(RT_BUFFER_OUTPUT);
    m_bufferDenoised->setFormat(RT_FORMAT_FLOAT4); // RGBA32F
    m_bufferDenoised->setSize(m_width, m_height);

#if USE_DENOISER_ALBEDO
    // Optional buffer for the denoiser. Improves denoising quality.
    m_bufferAlbedo  = m_context->createBuffer(RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT4, m_width, m_height);
    m_context["sysAlbedoBuffer"]->setBuffer(m_bufferAlbedo);

#if USE_DENOISER_NORMAL
    // Optional buffer for the denoiser. Improves denoising quality, but not as effective as the albedo.
    m_bufferNormals = m_context->createBuffer(RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT4, m_width, m_height);
    m_context["sysNormalBuffer"]->setBuffer(m_bufferNormals);
#endif
#endif

    m_stageDenoiser = m_context->createBuiltinPostProcessingStage("DLDenoiser");
    m_stageDenoiser->declareVariable("input_buffer");
    m_stageDenoiser->declareVariable("output_buffer");
#if USE_DENOISER_ALBEDO
    m_stageDenoiser->declareVariable("input_albedo_buffer");
#if USE_DENOISER_NORMAL
    m_stageDenoiser->declareVariable("input_normal_buffer");
#endif
#endif
    m_stageDenoiser->declareVariable("blend");  // The denoised image can be blended with the original input image with this variable.
    m_stageDenoiser->declareVariable("hdr");    // OptiX 5.1.0 supports HDR denoising which is shown in this example.
    //m_stageDenoiser->declareVariable("maxmem"); // OptiX 5.1.0 allows to limit the maximum amount of memory the DL Denoiser should use (in bytes).

    optix::Variable v = m_stageDenoiser->queryVariable("input_buffer");
    v->setBuffer(m_bufferOutput);
    v = m_stageDenoiser->queryVariable("output_buffer");
    v->setBuffer(m_bufferDenoised);
#if USE_DENOISER_ALBEDO
    v = m_stageDenoiser->queryVariable("input_albedo_buffer");
    v->setBuffer(m_bufferAlbedo);
#if USE_DENOISER_NORMAL
    v = m_stageDenoiser->queryVariable("input_normal_buffer");
    v->setBuffer(m_bufferNormals);
#endif
#endif
    v = m_stageDenoiser->queryVariable("blend");
    v->setFloat(m_denoiseBlend); // 0.0f means full denoised buffer, 1.0f means original input image.
    v = m_stageDenoiser->queryVariable("hdr");
    v->setUint(1); // Enable the HDR denoiser inside OptiX 5.1.0. "hdr" is an unsigned int variable. Non-zero means enabled.
    //v = m_stageDenoiser->queryVariable("maxmem");
    //v->setFloat(1024.0f * 1024.0f * 512.0f); // "maxmem" is a float variable [bytes]! Limit the maximum memory the denoiser should use.

    m_commandListDenoiser = m_context->createCommandList();

    m_commandListDenoiser->appendPostprocessingStage(m_stageDenoiser, m_width, m_height);
    m_commandListDenoiser->finalize();
#endif // USE_DENOISER
  }
  catch(optix::Exception& e)
  {
    std::cerr << e.getErrorString() << std::endl;
  }
}


void Application::initScene()
{
  try
  {
    m_timer.restart();
    const double timeInit = m_timer.getTime();

    std::cerr << "createScene()" << std::endl;
    createScene();
    const double timeScene = m_timer.getTime();

    std::cerr << "m_context->validate()" << std::endl;
    m_context->validate();
    const double timeValidate = m_timer.getTime();

    std::cerr << "m_context->launch()" << std::endl;
    m_context->launch(0, 0, 0); // Dummy launch to build everything (entrypoint, width, height)
    const double timeLaunch = m_timer.getTime();

    std::cerr << "initScene(): " << timeLaunch - timeInit << " seconds overall" << std::endl;
    std::cerr << "{" << std::endl;
    std::cerr << "  createScene() = " << timeScene    - timeInit     << " seconds" << std::endl;
    std::cerr << "  validate()    = " << timeValidate - timeScene    << " seconds" << std::endl;
    std::cerr << "  launch()      = " << timeLaunch   - timeValidate << " seconds" << std::endl;
    std::cerr << "}" << std::endl;
  }
  catch(optix::Exception& e)
  {
    std::cerr << e.getErrorString() << std::endl;
  }
}

void Application::restartAccumulation()
{
  m_iterationIndex  = 0;
  m_presentNext     = true;
  m_presentAtSecond = 1.0;

  m_timer.restart();
}


bool Application::render()
{
  bool repaint = false;

  try
  {
    optix::float3 cameraPosition;
    optix::float3 cameraU;
    optix::float3 cameraV;
    optix::float3 cameraW;

    bool cameraChanged = m_pinholeCamera.getFrustum(cameraPosition, cameraU, cameraV, cameraW);
    if (cameraChanged)
    {
      m_context["sysCameraPosition"]->setFloat(cameraPosition);
      m_context["sysCameraU"]->setFloat(cameraU);
      m_context["sysCameraV"]->setFloat(cameraV);
      m_context["sysCameraW"]->setFloat(cameraW);

      restartAccumulation();
    }

    // Continue manual accumulation rendering if there is no limit (m_frames == 0) or the number of frames has not been reached.
    if (0 == m_frames || m_iterationIndex < m_frames)
    {
      m_context["sysIterationIndex"]->setInt(m_iterationIndex); // Iteration index is zero-based!
      //m_context->launch(0, m_width, m_height);
      m_iterationIndex++;
    }

    // Only update the texture when a restart happened or one second passed to reduce required bandwidth.
    if (m_presentNext)
    {
#if USE_DENOISER
      m_commandListDenoiser->execute(); // Now the result is inside the m_denoisedBuffer.
#endif

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, m_hdrTexture); // Manual accumulation always renders into the m_hdrTexture.

#if USE_DENOISER
#if USE_DENOISER_ALBEDO
      // DAR DEBUG Show the albedo buffer.
      const void* data = m_bufferAlbedo->map(0, RT_BUFFER_MAP_READ);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (GLsizei) m_width, (GLsizei) m_height, 0, GL_RGBA, GL_FLOAT, data); // RGBA32F
      m_bufferAlbedo->unmap();

#if USE_DENOISER_NORMAL
      // DAR DEBUG Show the normal buffer.
      const void* data = m_bufferNormals->map(0, RT_BUFFER_MAP_READ);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (GLsizei) m_width, (GLsizei) m_height, 0, GL_RGBA, GL_FLOAT, data); // RGBA32F
      m_bufferNormals->unmap();
#endif
#endif

      if (m_interop)
      {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_bufferDenoised->getGLBOId());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (GLsizei) m_width, (GLsizei) m_height, 0, GL_RGBA, GL_FLOAT, (void*) 0); // RGBA32F from byte offset 0 in the pixel unpack buffer.
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
      }
      else
      {
        const void* data = m_bufferDenoised->map(0, RT_BUFFER_MAP_READ);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (GLsizei) m_width, (GLsizei) m_height, 0, GL_RGBA, GL_FLOAT, data); // RGBA32F
        m_bufferDenoised->unmap();
      }
#else
      if (m_interop)
      {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_bufferOutput->getGLBOId());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (GLsizei) m_width, (GLsizei) m_height, 0, GL_RGBA, GL_FLOAT, (void*) 0); // RGBA32F from byte offset 0 in the pixel unpack buffer.
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
      }
      else
      {
        const void* data = m_bufferOutput->map(0, RT_BUFFER_MAP_READ);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (GLsizei) m_width, (GLsizei) m_height, 0, GL_RGBA, GL_FLOAT, data); // RGBA32F
        m_bufferOutput->unmap();
      }
#endif // USE_DENOISER

      repaint = true; // Indicate that there is a new image.

      m_presentNext = m_present;
    }

    double seconds = m_timer.getTime();
#if 1
    // Show the accumulation of the first half second to remain interactive with "present 0" on the VCA.
    // Not done when benchmarking to get more accurate results.
    if (seconds < 0.5)
    {
      m_presentAtSecond = 1.0;
      m_presentNext     = true;
    }
    else
#endif
    if (m_presentAtSecond < seconds)
    {
      m_presentAtSecond = ceil(seconds);

      const double fps = double(m_iterationIndex) / seconds;

      std::ostringstream stream;
      stream.precision(3); // Precision is # digits in fraction part.
      // m_iterationIndex has already been incremented for the last rendered frame, so it is the actual framecount here.
      stream << std::fixed << m_iterationIndex << " / " << seconds << " = " << fps << " fps";
      std::cerr << stream.str() << std::endl;

      m_presentNext = true; // Present at least every second.
    }
  }
  catch(optix::Exception& e)
  {
    std::cerr << e.getErrorString() << std::endl;
  }
  return repaint;
}

void Application::display()
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_hdrTexture);

  glUseProgram(m_glslProgram);

  // the whole window is filled (black, if the surface hasn't been rendered)

  glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
  glEnd();

  glUseProgram(0);

  // // Trapezoid in middle of screen
  // glColor3f(0.0f, 1.0f, 0.0f); // sets color to green.
  // glBegin(GL_QUADS);
  //   glVertex2f(-0.25f, 0.25f); // vertex 1
  //   glVertex2f(-0.5f, -0.25f); // vertex 2
  //   glVertex2f(0.5f, -0.25f); // vertex 3
  //   glVertex2f(0.25f, 0.25f); // vertex 4
  // glEnd();


}

void Application::screenshot(std::string const& filename)
{
#if USE_DENOISER
  m_commandListDenoiser->execute(); // Must call the post-processing command list at least once to get the data into the denoised buffer.
  sutil::writeBufferToFile(filename.c_str(), m_bufferDenoised); // Store the denoised buffer!
#else
  sutil::writeBufferToFile(filename.c_str(), m_bufferOutput);
#endif
  std::cerr << "Wrote " << filename << std::endl;
}

// Helper functions:
void Application::checkInfoLog(const char *msg, GLuint object)
{
  GLint maxLength;
  GLint length;
  GLchar *infoLog;

  if (glIsProgram(object))
  {
    glGetProgramiv(object, GL_INFO_LOG_LENGTH, &maxLength);
  }
  else
  {
    glGetShaderiv(object, GL_INFO_LOG_LENGTH, &maxLength);
  }
  if (maxLength > 1)
  {
    infoLog = (GLchar *) malloc(maxLength);
    if (infoLog != NULL)
    {
      if (glIsShader(object))
      {
        glGetShaderInfoLog(object, maxLength, &length, infoLog);
      }
      else
      {
        glGetProgramInfoLog(object, maxLength, &length, infoLog);
      }
      // fprintf(fileLog, "-- tried to compile (len=%d): %s\n", (unsigned int)strlen(msg), msg);
      // fprintf(fileLog, "--- info log contents (len=%d) ---\n", (int) maxLength);
      // fprintf(fileLog, "%s", infoLog);
      // fprintf(fileLog, "--- end ---\n");
      std::cerr << msg << std::endl;
      std::cerr << infoLog << std::endl;
      // Look at the info log string here...
      free(infoLog);
    }
  }
}


void Application::initGLSL()
{
  static const std::string vsSource =
    "#version 330\n"
    "layout(location = 0) in vec4 attrPosition;\n"
    "layout(location = 8) in vec2 attrTexCoord0;\n"
    "out vec2 varTexCoord0;\n"
    "void main()\n"
    "{\n"
    "  gl_Position  = attrPosition;\n"
    "  varTexCoord0 = attrTexCoord0;\n"
    "}\n";

  static const std::string fsSource =
    "#version 330\n"
    "uniform sampler2D samplerHDR;\n"
    "uniform vec3  colorBalance;\n"
    "uniform float invWhitePoint;\n"
    "uniform float burnHighlights;\n"
    "uniform float saturation;\n"
    "uniform float crushBlacks;\n"
    "uniform float invGamma;\n"
    "in vec2 varTexCoord0;\n"
    "layout(location = 0, index = 0) out vec4 outColor;\n"
    "void main()\n"
    "{\n"
    "  vec3 hdrColor = texture(samplerHDR, varTexCoord0).rgb;\n"
    "  vec3 ldrColor = invWhitePoint * colorBalance * hdrColor;\n"
    "  ldrColor *= (ldrColor * burnHighlights + 1.0) / (ldrColor + 1.0);\n"
    "  float luminance = dot(ldrColor, vec3(0.3, 0.59, 0.11));\n"
    "  ldrColor = max(mix(vec3(luminance), ldrColor, saturation), 0.0);\n"
    "  luminance = dot(ldrColor, vec3(0.3, 0.59, 0.11));\n"
    "  if (luminance < 1.0)\n"
    "  {\n"
    "    ldrColor = max(mix(pow(ldrColor, vec3(crushBlacks)), ldrColor, sqrt(luminance)), 0.0);\n"
    "  }\n"
    "  ldrColor = pow(ldrColor, vec3(invGamma));\n"
    "  outColor = vec4(ldrColor, 1.0);\n"
    "}\n";

  GLint vsCompiled = 0;
  GLint fsCompiled = 0;

  m_glslVS = glCreateShader(GL_VERTEX_SHADER);
  if (m_glslVS)
  {
    GLsizei len = (GLsizei) vsSource.size();
    const GLchar *vs = vsSource.c_str();
    glShaderSource(m_glslVS, 1, &vs, &len);
    glCompileShader(m_glslVS);
    checkInfoLog(vs, m_glslVS);

    glGetShaderiv(m_glslVS, GL_COMPILE_STATUS, &vsCompiled);
    MY_ASSERT(vsCompiled);
  }

  m_glslFS = glCreateShader(GL_FRAGMENT_SHADER);
  if (m_glslFS)
  {
    GLsizei len = (GLsizei) fsSource.size();
    const GLchar *fs = fsSource.c_str();
    glShaderSource(m_glslFS, 1, &fs, &len);
    glCompileShader(m_glslFS);
    checkInfoLog(fs, m_glslFS);

    glGetShaderiv(m_glslFS, GL_COMPILE_STATUS, &fsCompiled);
    MY_ASSERT(fsCompiled);
  }

  m_glslProgram = glCreateProgram();
  if (m_glslProgram)
  {
    GLint programLinked = 0;

    if (m_glslVS && vsCompiled)
    {
      glAttachShader(m_glslProgram, m_glslVS);
    }
    if (m_glslFS && fsCompiled)
    {
      glAttachShader(m_glslProgram, m_glslFS);
    }

    glLinkProgram(m_glslProgram);
    checkInfoLog("m_glslProgram", m_glslProgram);

    glGetProgramiv(m_glslProgram, GL_LINK_STATUS, &programLinked);
    MY_ASSERT(programLinked);

    if (programLinked)
    {
      glUseProgram(m_glslProgram);

      glUniform1i(glGetUniformLocation(m_glslProgram, "samplerHDR"), 0); // texture image unit 0

      glUniform1f(glGetUniformLocation(m_glslProgram, "invGamma"), 1.0f / m_gamma);
      glUniform3f(glGetUniformLocation(m_glslProgram, "colorBalance"), m_colorBalance.x, m_colorBalance.y, m_colorBalance.z);
      glUniform1f(glGetUniformLocation(m_glslProgram, "invWhitePoint"), m_brightness / m_whitePoint);
      glUniform1f(glGetUniformLocation(m_glslProgram, "burnHighlights"), m_burnHighlights);
      glUniform1f(glGetUniformLocation(m_glslProgram, "crushBlacks"), m_crushBlacks + m_crushBlacks + 1.0f);
      glUniform1f(glGetUniformLocation(m_glslProgram, "saturation"), m_saturation);

      glUseProgram(0);
    }
  }
}


void Application::guiWindow()
{
  if (!m_isWindowVisible) // Use SPACE to toggle the display of the GUI window.
  {
    return;
  }

  ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);  // changed in v1.51

  ImGuiWindowFlags window_flags = 0;
  if (!ImGui::Begin("optixGui", nullptr, window_flags)) // No bool flag to omit the close button.
  {
    // Early out if the window is collapsed, as an optimization.
    ImGui::End();
    return;
  }

  ImGui::PushItemWidth(-100); // right-aligned, keep 180 pixels for the labels.

  if (ImGui::CollapsingHeader("System"))
  {
    if (ImGui::Checkbox("Present", &m_present))
    {
      // No action needed, happens automatically.
    }
    if (ImGui::Combo("Camera", (int*) &m_cameraType, "Pinhole\0Fisheye\0Spherical\0\0"))
    {
      m_context["sysCameraType"]->setInt(m_cameraType);
      restartAccumulation();
    }
    if (ImGui::Combo("Shutter", &m_shutterType, "Stochastic\0Top to Bottom\0Bottom To Top\0Left To Right\0Right To Left\0\0"))
    {
      m_context["sysShutterType"]->setInt(m_shutterType);
      restartAccumulation();
    }
    if (ImGui::DragInt("Min Paths", &m_minPathLength, 1.0f, 0, 100))
    {
      m_context["sysPathLengths"]->setInt(m_minPathLength, m_maxPathLength);
      restartAccumulation();
    }
    if (ImGui::DragInt("Max Paths", &m_maxPathLength, 1.0f, 0, 100))
    {
      m_context["sysPathLengths"]->setInt(m_minPathLength, m_maxPathLength);
      restartAccumulation();
    }
    if (ImGui::DragFloat("Scene Epsilon", &m_sceneEpsilonFactor, 1.0f, 0.0f, 10000.0f))
    {
      m_context["sysSceneEpsilon"]->setFloat(m_sceneEpsilonFactor * 1e-7f);
      restartAccumulation();
    }
    if (ImGui::DragFloat("Env Rotation", &m_environmentRotation, 0.001f, 0.0f, 1.0f))
    {
      m_context["sysEnvironmentRotation"]->setFloat(m_environmentRotation);
      restartAccumulation();
    }
#if USE_DENOISER
    if (ImGui::DragFloat("Denoise Blend", &m_denoiseBlend, 0.01f, 0.0f, 1.0f, "%.2f"))
    {
      optix::Variable v = m_stageDenoiser->queryVariable("blend");
      v->setFloat(m_denoiseBlend);
    }
#endif
    if (ImGui::DragInt("Frames", &m_frames, 1.0f, 0, 10000))
    {
      if (m_frames != 0 && m_frames < m_iterationIndex) // If we already rendered more frames, start again.
      {
        restartAccumulation();
      }
    }
    if (ImGui::DragFloat("Mouse Ratio", &m_mouseSpeedRatio, 0.1f, 0.1f, 1000.0f, "%.1f"))
    {
      m_pinholeCamera.setSpeedRatio(m_mouseSpeedRatio);
    }
  }
  if (ImGui::CollapsingHeader("Tonemapper"))
  {
    if (ImGui::ColorEdit3("Balance", (float*) &m_colorBalance))
    {
      glUseProgram(m_glslProgram);
      glUniform3f(glGetUniformLocation(m_glslProgram, "colorBalance"), m_colorBalance.x, m_colorBalance.y, m_colorBalance.z);
      glUseProgram(0);
    }
    if (ImGui::DragFloat("Gamma", &m_gamma, 0.01f, 0.01f, 10.0f)) // Must not get 0.0f
    {
      glUseProgram(m_glslProgram);
      glUniform1f(glGetUniformLocation(m_glslProgram, "invGamma"), 1.0f / m_gamma);
      glUseProgram(0);
    }
    if (ImGui::DragFloat("White Point", &m_whitePoint, 0.01f, 0.01f, 255.0f, "%.2f", 2.0f)) // Must not get 0.0f
    {
      glUseProgram(m_glslProgram);
      glUniform1f(glGetUniformLocation(m_glslProgram, "invWhitePoint"), m_brightness / m_whitePoint);
      glUseProgram(0);
    }
    if (ImGui::DragFloat("Burn Lights", &m_burnHighlights, 0.01f, 0.0f, 10.0f, "%.2f"))
    {
      glUseProgram(m_glslProgram);
      glUniform1f(glGetUniformLocation(m_glslProgram, "burnHighlights"), m_burnHighlights);
      glUseProgram(0);
    }
    if (ImGui::DragFloat("Crush Blacks", &m_crushBlacks, 0.01f, 0.0f, 1.0f, "%.2f"))
    {
      glUseProgram(m_glslProgram);
      glUniform1f(glGetUniformLocation(m_glslProgram, "crushBlacks"),  m_crushBlacks + m_crushBlacks + 1.0f);
      glUseProgram(0);
    }
    if (ImGui::DragFloat("Saturation", &m_saturation, 0.01f, 0.0f, 10.0f, "%.2f"))
    {
      glUseProgram(m_glslProgram);
      glUniform1f(glGetUniformLocation(m_glslProgram, "saturation"), m_saturation);
      glUseProgram(0);
    }
    if (ImGui::DragFloat("Brightness", &m_brightness, 0.01f, 0.0f, 100.0f, "%.2f", 2.0f))
    {
      glUseProgram(m_glslProgram);
      glUniform1f(glGetUniformLocation(m_glslProgram, "invWhitePoint"), m_brightness / m_whitePoint);
      glUseProgram(0);
    }
  }
  if (ImGui::CollapsingHeader("Materials"))
  {
    bool changed = false;

    for (int i = 0; i < int(m_guiMaterialParameters.size()); ++i)
    {
      if (ImGui::TreeNode((void*)(intptr_t) i, "Material %d", i))
      {
        MaterialParameterGUI& parameters = m_guiMaterialParameters[i];

        if (ImGui::Combo("BSDF Type", (int*) &parameters.indexBSDF,
                         "Diffuse Reflection\0Specular Reflection\0Specular Reflection Transmission\0\0"))
        {
          changed = true;
        }
        if (ImGui::ColorEdit3("Albedo", (float*) &parameters.albedo))
        {
          changed = true;
        }
        if (ImGui::Checkbox("Use Albedo Texture", &parameters.useAlbedoTexture))
        {
          changed = true;
        }
        if (ImGui::Checkbox("Thin-Walled", &parameters.thinwalled)) // Set this to true when using cutout opacity!
        {
          changed = true;
        }
        // Only show material parameters for the BSDFs which are affected.
        if (parameters.indexBSDF == INDEX_BSDF_SPECULAR_REFLECTION_TRANSMISSION)
        {
          if (ImGui::ColorEdit3("Absorption", (float*) &parameters.absorptionColor))
          {
            changed = true;
          }
          if (ImGui::DragFloat("Volume Scale", &parameters.volumeDistanceScale, 0.01f, 0.0f, 100.0f, "%.2f"))
          {
            changed = true;
          }
          if (ImGui::DragFloat("IOR", &parameters.ior, 0.01f, 0.0f, 10.0f, "%.2f"))
          {
            changed = true;
          }
        }
        ImGui::TreePop();
      }
    }

    if (changed) // If any of the material parameters changed, simply upload them to the sysMaterialParameters again.
    {
      updateMaterialParameters();
      restartAccumulation();
    }
  }
  if (ImGui::CollapsingHeader("Lights"))
  {
    bool changed = false;

    for (int i = 0; i < int(m_lightDefinitions.size()); ++i)
    {
      LightDefinition& light = m_lightDefinitions[i];

      // Allow to change the emission (radiant exitance in Watt/m^2 of the rectangle lights in the scene.
      if (light.type == LIGHT_PARALLELOGRAM)
      {
        if (ImGui::TreeNode((void*)(intptr_t) i, "Light %d", i))
        {
          if (ImGui::DragFloat3("Emission", (float*) &light.emission, 1.0f, 0.0f, 10000.0f, "%.0f"))
          {
            changed = true;
          }
          ImGui::TreePop();
        }
      }
    }
    if (changed) // If any of the light parameters changed, simply upload them to the sysMaterialParameters again.
    {
      // Upload the light definitions into the sysLightDefinitions buffer.
      void* dst = static_cast<LightDefinition*>(m_bufferLightDefinitions->map(0, RT_BUFFER_MAP_WRITE_DISCARD));
      memcpy(dst, m_lightDefinitions.data(), sizeof(LightDefinition) * m_lightDefinitions.size());
      m_bufferLightDefinitions->unmap();

      restartAccumulation();
    }
  }

  ImGui::PopItemWidth();

  ImGui::End();
}

void Application::guiEventHandler()
{
  ImGuiIO const& io = ImGui::GetIO();

  if (ImGui::IsKeyPressed(' ', false)) // Toggle the GUI window display with SPACE key.
  {
    m_isWindowVisible = !m_isWindowVisible;
  }

  const ImVec2 mousePosition = ImGui::GetMousePos(); // Mouse coordinate window client rect.
  const int x = int(mousePosition.x);
  const int y = int(mousePosition.y);

  switch (m_guiState) // missing GUI_STATE_FOCUS
  {
    case GUI_STATE_NONE:
      if (!io.WantCaptureMouse) // Only allow camera interactions to begin when interacting with the GUI.
      {
        if (ImGui::IsMouseDown(0)) // LMB down event?
        {
          m_pinholeCamera.setBaseCoordinates(x, y);
          m_guiState = GUI_STATE_ORBIT;
        }
        else if (ImGui::IsMouseDown(1)) // RMB down event?
        {
          m_pinholeCamera.setBaseCoordinates(x, y);
          m_guiState = GUI_STATE_DOLLY;
        }
        else if (ImGui::IsMouseDown(2)) // MMB down event?
        {
          m_pinholeCamera.setBaseCoordinates(x, y);
          m_guiState = GUI_STATE_PAN;
        }
        else if (io.MouseWheel != 0.0f) // Mouse wheel zoom.
        {
          m_pinholeCamera.zoom(io.MouseWheel);
        }
      }
      break;

    case GUI_STATE_ORBIT:
      if (ImGui::IsMouseReleased(0)) // LMB released? End of orbit mode.
      {
        m_guiState = GUI_STATE_NONE;
      }
      else
      {
        m_pinholeCamera.orbit(x, y);
      }
      break;

    case GUI_STATE_DOLLY:
      if (ImGui::IsMouseReleased(1)) // RMB released? End of dolly mode.
      {
        m_guiState = GUI_STATE_NONE;
      }
      else
      {
        m_pinholeCamera.dolly(x, y);
      }
      break;

    case GUI_STATE_PAN:
      if (ImGui::IsMouseReleased(2)) // MMB released? End of pan mode.
      {
        m_guiState = GUI_STATE_NONE;
      }
      else
      {
        m_pinholeCamera.pan(x, y);
      }
      break;
  }
}


// This part is always identical in the generated geometry creation routines.
optix::Geometry Application::createGeometry(std::vector<VertexAttributes> const& attributes, std::vector<unsigned int> const& indices)
{
  optix::Geometry geometry(nullptr);

  try
  {
    geometry = m_context->createGeometry();

    optix::Buffer attributesBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_USER);
    attributesBuffer->setElementSize(sizeof(VertexAttributes));
    attributesBuffer->setSize(attributes.size());

    void *dst = attributesBuffer->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
    memcpy(dst, attributes.data(), sizeof(VertexAttributes) * attributes.size());
    attributesBuffer->unmap();

    optix::Buffer indicesBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_INT3, indices.size() / 3);
    dst = indicesBuffer->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
    memcpy(dst, indices.data(), sizeof(optix::uint3) * indices.size() / 3);
    indicesBuffer->unmap();

    std::map<std::string, optix::Program>::const_iterator it = m_mapOfPrograms.find("boundingbox_triangle_indexed");
    MY_ASSERT(it != m_mapOfPrograms.end());
    geometry->setBoundingBoxProgram(it->second);

    it = m_mapOfPrograms.find("intersection_triangle_indexed");
    MY_ASSERT(it != m_mapOfPrograms.end());
    geometry->setIntersectionProgram(it->second);

    geometry["attributesBuffer"]->setBuffer(attributesBuffer);
    geometry["indicesBuffer"]->setBuffer(indicesBuffer);
    geometry->setPrimitiveCount((unsigned int)(indices.size()) / 3);
  }
  catch(optix::Exception& e)
  {
    std::cerr << e.getErrorString() << std::endl;
  }
  return geometry;
}


void Application::initPrograms()
{
  try
  {
    // First load all programs and put them into a map.
    // Programs which are reused multiple times can be queried from that map.
    // (This renderer does not put variables on program scope!)

    // // Renderer
    // m_mapOfPrograms["raygeneration"] = m_context->createProgramFromPTXFile(ptxPath("raygeneration.cu"), "raygeneration"); // entry point 0
    // m_mapOfPrograms["exception"]     = m_context->createProgramFromPTXFile(ptxPath("exception.cu"), "exception"); // entry point 0

    // // There can be only one of the miss programs active.
    // switch (m_missID)
    // {
    // case 0: // Default black environment. Does not appear in the light definitions, means it's not used in direct lighting.
    //   m_mapOfPrograms["miss"] = m_context->createProgramFromPTXFile(ptxPath("miss.cu"), "miss_environment_null"); // ray type 0
    //   break;
    // case 1:
    // default:
    //   m_mapOfPrograms["miss"] = m_context->createProgramFromPTXFile(ptxPath("miss.cu"), "miss_environment_constant"); // raytype 0
    //   break;
    // case 2:
    //   m_mapOfPrograms["miss"] = m_context->createProgramFromPTXFile(ptxPath("miss.cu"), "miss_environment_mapping"); // raytype 0
    //   break;
    // }

    // // Geometry
    // m_mapOfPrograms["boundingbox_triangle_indexed"]  = m_context->createProgramFromPTXFile(ptxPath("boundingbox_triangle_indexed.cu"),  "boundingbox_triangle_indexed");
    // m_mapOfPrograms["intersection_triangle_indexed"] = m_context->createProgramFromPTXFile(ptxPath("intersection_triangle_indexed.cu"), "intersection_triangle_indexed");

    // // Material programs. There are only three Material nodes, opaque, cutout opacity and rectangle lights.
    // // For the radiance ray type 0:
    // m_mapOfPrograms["closesthit"]       = m_context->createProgramFromPTXFile(ptxPath("closesthit.cu"), "closesthit");
    // m_mapOfPrograms["closesthit_light"] = m_context->createProgramFromPTXFile(ptxPath("closesthit_light.cu"), "closesthit_light");
    // m_mapOfPrograms["anyhit_cutout"]    = m_context->createProgramFromPTXFile(ptxPath("anyhit.cu"), "anyhit_cutout");
    // // For the shadow ray type 1:
    // m_mapOfPrograms["anyhit_shadow"]        = m_context->createProgramFromPTXFile(ptxPath("anyhit.cu"), "anyhit_shadow");        // Opaque
    // m_mapOfPrograms["anyhit_shadow_cutout"] = m_context->createProgramFromPTXFile(ptxPath("anyhit.cu"), "anyhit_shadow_cutout"); // Cutout opacity.

    // Now setup all buffers of bindless callable program IDs.
    // These are device side function tables which can be indexed at runtime without recompilation.

    // Different lens shader implementations as bindless callable program IDs inside "sysLensShader".
    m_bufferLensShader = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_PROGRAM_ID, 3);
    int* lensShader = (int*) m_bufferLensShader->map(0, RT_BUFFER_MAP_WRITE_DISCARD);

    // const std::string ptxPathLensShader = ptxPath("lens_shader.cu");
    // optix::Program prg = m_context->createProgramFromPTXFile(ptxPathLensShader, "lens_shader_pinhole");
    // m_mapOfPrograms["lens_shader_pinhole"] = prg;
    // lensShader[LENS_SHADER_PINHOLE] = prg->getId();

    // prg = m_context->createProgramFromPTXFile(ptxPathLensShader, "lens_shader_fisheye");
    // m_mapOfPrograms["lens_shader_fisheye"] = prg;
    // lensShader[LENS_SHADER_FISHEYE] = prg->getId();

    // prg = m_context->createProgramFromPTXFile(ptxPathLensShader, "lens_shader_sphere");
    // m_mapOfPrograms["lens_shader_sphere"] = prg;
    // lensShader[LENS_SHADER_SPHERE] = prg->getId();

    m_bufferLensShader->unmap();

    m_context["sysLensShader"]->setBuffer(m_bufferLensShader);

    // PERF One possible optimization to reduce the OptiX kernel size even more
    // is to only download the programs for materials actually present in the scene. Not done in this demo.

    // BSDF sampling functions.
    m_bufferSampleBSDF = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_PROGRAM_ID, NUMBER_OF_BSDF_INDICES);
    int* sampleBsdf = (int*) m_bufferSampleBSDF->map(0, RT_BUFFER_MAP_WRITE_DISCARD);

    // prg = m_context->createProgramFromPTXFile(ptxPath("bsdf_diffuse_reflection.cu"), "sample_bsdf_diffuse_reflection");
    // m_mapOfPrograms["sample_bsdf_diffuse_reflection"] = prg;
    // sampleBsdf[INDEX_BSDF_DIFFUSE_REFLECTION] = prg->getId();

    // prg = m_context->createProgramFromPTXFile(ptxPath("bsdf_specular_reflection.cu"), "sample_bsdf_specular_reflection");
    // m_mapOfPrograms["sample_bsdf_specular_reflection"] = prg;
    // sampleBsdf[INDEX_BSDF_SPECULAR_REFLECTION] = prg->getId();

    // prg = m_context->createProgramFromPTXFile(ptxPath("bsdf_specular_reflection_transmission.cu"), "sample_bsdf_specular_reflection_transmission");
    // m_mapOfPrograms["sample_bsdf_specular_reflection_transmission"] = prg;
    // sampleBsdf[INDEX_BSDF_SPECULAR_REFLECTION_TRANSMISSION] = prg->getId();

    m_bufferSampleBSDF->unmap();

    m_context["sysSampleBSDF"]->setBuffer(m_bufferSampleBSDF);

    // BSDF evaluation functions
    m_bufferEvalBSDF = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_PROGRAM_ID, NUMBER_OF_BSDF_INDICES);
    int* evalBsdf = (int*) m_bufferEvalBSDF->map(0, RT_BUFFER_MAP_WRITE_DISCARD);

    // prg = m_context->createProgramFromPTXFile(ptxPath("bsdf_diffuse_reflection.cu"), "eval_bsdf_diffuse_reflection");
    // m_mapOfPrograms["eval_bsdf_diffuse_reflection"] = prg;
    // evalBsdf[INDEX_BSDF_DIFFUSE_REFLECTION] = prg->getId();

    // prg = m_context->createProgramFromPTXFile(ptxPath("bsdf_specular_reflection.cu"), "eval_bsdf_specular_reflection");
    // m_mapOfPrograms["eval_bsdf_specular_reflection"] = prg;
    // evalBsdf[INDEX_BSDF_SPECULAR_REFLECTION]              = prg->getId(); // All specular evaluation functions just returns float4(0.0f).
    // evalBsdf[INDEX_BSDF_SPECULAR_REFLECTION_TRANSMISSION] = prg->getId(); // Reuse the same program for all specular materials to keep the kernel small.

    m_bufferEvalBSDF->unmap();

    m_context["sysEvalBSDF"]->setBuffer(m_bufferEvalBSDF);

    // Light sampling functions.
    // There are only two light types implemented in this renderer: environment lights and parallelogram area lights.
    m_bufferSampleLight = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_PROGRAM_ID, 2);
    int* sampleLight = (int*) m_bufferSampleLight->map(0, RT_BUFFER_MAP_WRITE_DISCARD);

    sampleLight[LIGHT_ENVIRONMENT]   = RT_PROGRAM_ID_NULL;
    sampleLight[LIGHT_PARALLELOGRAM] = RT_PROGRAM_ID_NULL;

    switch (m_missID)
    {
    case 0: // Default black environment. // PERF This is not a light, means it doesn't appear in the sysLightDefinitions!
    default:
      break;
    // case 1:
    //   prg = m_context->createProgramFromPTXFile(ptxPath("light_sample.cu"), "sample_light_constant");
    //   m_mapOfPrograms["sample_light_constant"] = prg;
    //   sampleLight[LIGHT_ENVIRONMENT] = prg->getId();
    //   break;
    // case 2:
    //   prg = m_context->createProgramFromPTXFile(ptxPath("light_sample.cu"), "sample_light_environment");
    //   m_mapOfPrograms["sample_light_environment"] = prg;
    //   sampleLight[LIGHT_ENVIRONMENT] = prg->getId();
    //   break;
    }

    // // PERF Again, to optimize the kernel size this program would only be needed if there are parallelogram lights in the scene.
    // prg = m_context->createProgramFromPTXFile(ptxPath("light_sample.cu"), "sample_light_parallelogram");
    // m_mapOfPrograms["sample_light_parallelogram"] = prg;
    // sampleLight[LIGHT_PARALLELOGRAM] = prg->getId();

    m_bufferSampleLight->unmap();

    m_context["sysSampleLight"]->setBuffer(m_bufferSampleLight);
  }
  catch(optix::Exception& e)
  {
    std::cerr << e.getErrorString() << std::endl;
  }
}

void Application::updateMaterialParameters()
{
  MY_ASSERT((sizeof(MaterialParameter) & 15) == 0); // Verify float4 alignment.

  // Convert the GUI material parameters to the device side structure and upload them into the context global buffer.
  MaterialParameter* dst = static_cast<MaterialParameter*>(m_bufferMaterialParameters->map(0, RT_BUFFER_MAP_WRITE_DISCARD));

  for (size_t i = 0; i < m_guiMaterialParameters.size(); ++i, ++dst)
  {
    MaterialParameterGUI& src = m_guiMaterialParameters[i];

    dst->indexBSDF = src.indexBSDF;
    dst->albedo     = src.albedo;
    dst->albedoID   = (src.useAlbedoTexture) ? m_textureAlbedo.getId() : RT_TEXTURE_ID_NULL;
    dst->cutoutID   = (src.useCutoutTexture) ? m_textureCutout.getId() : RT_TEXTURE_ID_NULL;
    dst->flags      = (src.thinwalled) ? FLAG_THINWALLED : 0;
    // Calculate the effective absorption coefficient from the GUI parameters. This is one reason why there are two structures.
    // Prevent logf(0.0f) which results in infinity.
    const float x = (0.0f < src.absorptionColor.x) ? -logf(src.absorptionColor.x) : RT_DEFAULT_MAX;
    const float y = (0.0f < src.absorptionColor.y) ? -logf(src.absorptionColor.y) : RT_DEFAULT_MAX;
    const float z = (0.0f < src.absorptionColor.z) ? -logf(src.absorptionColor.z) : RT_DEFAULT_MAX;
    dst->absorption = optix::make_float3(x, y, z) * src.volumeDistanceScale;
    dst->ior = src.ior;
  }

  m_bufferMaterialParameters->unmap();
}

void Application::initMaterials()
{
  Picture* picture = new Picture;

  std::string textureFilename = std::string(sutil::samplesDir()) + "/data/NVIDIA_logo.jpg";
  picture->load(textureFilename);
  m_textureAlbedo.createSampler(m_context, picture);

  textureFilename = std::string(sutil::samplesDir()) + "/data/slots_alpha.png";
  picture->load(textureFilename);
  m_textureCutout.createSampler(m_context, picture);

  delete picture;

  // Setup GUI material parameters, one for each of the implemented BSDFs.
  // Cutout opacity is not an option which can be switched dynamically in this demo.
  // That would require to use the anyhit cutout version for all materials which is a performance impact.
  // Currently only the object which uses this material parameter instance has the cutout opacity assigned.

  // It's recommended that cutout opacity materials are always thin-walled.
  // Transparent volumetric materials would look strange otherwise.

  MaterialParameterGUI parameters;

  // Lambert material (for the floor)
  parameters.indexBSDF           = INDEX_BSDF_DIFFUSE_REFLECTION; // Index into sysSampleBSDF and sysEvalBSDF.
  parameters.albedo              = optix::make_float3(0.5f); // Grey. (Modulates the albedo texture.)
  parameters.useAlbedoTexture    = true; // Enabled just to distinguish the resulting image from the non HDR DL Denoiser used in optixIntro_09.
  parameters.useCutoutTexture    = false;
  parameters.thinwalled          = false;
  parameters.absorptionColor     = optix::make_float3(1.0f);
  parameters.volumeDistanceScale = 1.0f;
  parameters.ior                 = 1.5f;
  m_guiMaterialParameters.push_back(parameters); // 0

  // Lambert material with cutout opacity.
  parameters.indexBSDF           = INDEX_BSDF_DIFFUSE_REFLECTION;
  parameters.albedo              = optix::make_float3(0.6f, 0.0f, 0.0f); // Red.
  parameters.useAlbedoTexture    = false;
  parameters.useCutoutTexture    = true;
  parameters.thinwalled          = true; // Materials with cutout opacity should always be thinwalled.
  parameters.absorptionColor     = optix::make_float3(0.25f);
  parameters.volumeDistanceScale = 1.0f;
  parameters.ior                 = 1.5f;
  m_guiMaterialParameters.push_back(parameters); // 1

  // Water material.
  parameters.indexBSDF           = INDEX_BSDF_SPECULAR_REFLECTION_TRANSMISSION;
  parameters.albedo              = optix::make_float3(1.0f);
  parameters.useAlbedoTexture    = false;
  parameters.useCutoutTexture    = false;
  parameters.thinwalled          = false;
  parameters.absorptionColor     = optix::make_float3(0.980392f, 0.729412f, 0.470588f); // My favorite test color.
  parameters.volumeDistanceScale = 1.0f;
  parameters.ior                 = 1.33f; // Water
  m_guiMaterialParameters.push_back(parameters); // 2

  // Tinted mirror material.
  parameters.indexBSDF           = INDEX_BSDF_SPECULAR_REFLECTION;
  parameters.albedo              = optix::make_float3(0.2f, 0.2f, 0.8f); // Not full primary color to get nice HDR highlights.
  parameters.useAlbedoTexture    = false;
  parameters.useCutoutTexture    = false;
  parameters.thinwalled          = false;
  parameters.absorptionColor     = optix::make_float3(0.6f, 0.6f, 0.8f);
  parameters.volumeDistanceScale = 1.0f;
  parameters.ior                 = 1.33f;
  m_guiMaterialParameters.push_back(parameters); // 3

  try
  {
    m_bufferMaterialParameters = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_USER);
    m_bufferMaterialParameters->setElementSize(sizeof(MaterialParameter));
    m_bufferMaterialParameters->setSize(m_guiMaterialParameters.size()); // As many as there are in the GUI.

    updateMaterialParameters();

    m_context["sysMaterialParameters"]->setBuffer(m_bufferMaterialParameters);

    // Create the three main Material nodes to have the matching closest hit and any hit programs.

    std::map<std::string, optix::Program>::const_iterator it;

    // Used for all materials without cutout opacity. (Faster than using the cutout opacity material for everything.)
    m_opaqueMaterial = m_context->createMaterial();
    it = m_mapOfPrograms.find("closesthit");
    MY_ASSERT(it != m_mapOfPrograms.end());
    m_opaqueMaterial->setClosestHitProgram(0, it->second); // raytype radiance

    it = m_mapOfPrograms.find("anyhit_shadow");
    MY_ASSERT(it != m_mapOfPrograms.end());
    m_opaqueMaterial->setAnyHitProgram(1, it->second); // raytype shadow

    // Used for all materials with cutout opacity.
    m_cutoutMaterial = m_context->createMaterial();
    it = m_mapOfPrograms.find("closesthit");
    MY_ASSERT(it != m_mapOfPrograms.end());
    m_cutoutMaterial->setClosestHitProgram(0, it->second); // raytype radiance

    it = m_mapOfPrograms.find("anyhit_cutout");
    MY_ASSERT(it != m_mapOfPrograms.end());
    m_cutoutMaterial->setAnyHitProgram(0, it->second); // raytype radiance

    it = m_mapOfPrograms.find("anyhit_shadow_cutout");
    MY_ASSERT(it != m_mapOfPrograms.end());
    m_cutoutMaterial->setAnyHitProgram(1, it->second); // raytype shadow

    // Used for all geometric lights.
    m_lightMaterial = m_context->createMaterial();
    it = m_mapOfPrograms.find("closesthit_light"); // Special cased closest hit program for rectangle lights, diffuse EDF.
    MY_ASSERT(it != m_mapOfPrograms.end());
    m_lightMaterial->setClosestHitProgram(0, it->second); // raytype radiance

    it = m_mapOfPrograms.find("anyhit_shadow"); // Paralellogram area lights are opaque and throw shadows from other lights.
    MY_ASSERT(it != m_mapOfPrograms.end());
    m_lightMaterial->setAnyHitProgram(1, it->second); // raytype shadow
  }
  catch(optix::Exception& e)
  {
    std::cerr << e.getErrorString() << std::endl;
  }
}


// Scene testing all materials on a single geometry instanced via transforms and sharing one acceleration structure.
void Application::createScene()
{
  initMaterials();

  try
  {
    // OptiX Scene Graph construction.
    m_rootAcceleration = m_context->createAcceleration(m_builder); // No need to set acceleration properties on the top level Acceleration.

    m_rootGroup = m_context->createGroup(); // The scene's root group nodes becomes the sysTopObject.
    m_rootGroup->setAcceleration(m_rootAcceleration);

    m_context["sysTopObject"]->set(m_rootGroup); // This is where the rtTrace calls start the BVH traversal. (Same for radiance and shadow rays.)

    unsigned int count;

    // Demo code only!
    // Mind that these local OptiX objects will leak when not cleaning up the scene properly on changes.
    // Destroying the OptiX context will clean them up at program exit though.

    // Add a ground plane on the xz-plane at y = 0.0f.
    optix::Geometry geoPlane = createPlane(1, 1, 1);

    optix::GeometryInstance giPlane = m_context->createGeometryInstance(); // This connects Geometries with Materials.
    giPlane->setGeometry(geoPlane);
    giPlane->setMaterialCount(1);
    giPlane->setMaterial(0, (m_guiMaterialParameters[0].useCutoutTexture) ? m_cutoutMaterial : m_opaqueMaterial);
    giPlane["parMaterialIndex"]->setInt(0); // This is all! This defines which material parameters in sysMaterialParameters to use.

    optix::Acceleration accPlane = m_context->createAcceleration(m_builder);
    setAccelerationProperties(accPlane);

    optix::GeometryGroup ggPlane = m_context->createGeometryGroup(); // This connects GeometryInstances with Acceleration structures. (All OptiX nodes with "Group" in the name hold an Acceleration.)
    ggPlane->setAcceleration(accPlane);
    ggPlane->setChildCount(1);
    ggPlane->setChild(0, giPlane);

    // Scale the plane to go from -8 to 8.
    float trafoPlane[16] =
    {
      8.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 8.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 8.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };
    optix::Matrix4x4 matrixPlane(trafoPlane);

    optix::Transform trPlane = m_context->createTransform();
    trPlane->setChild(ggPlane);
    trPlane->setMatrix(false, matrixPlane.getData(), matrixPlane.inverse().getData());

    count = m_rootGroup->getChildCount();
    m_rootGroup->setChildCount(count + 1);
    m_rootGroup->setChild(count, trPlane);

    // Add a box (no tessellation here, using just 12 triangles)
    optix::Geometry geoBox = createBox();

    optix::GeometryInstance giBox = m_context->createGeometryInstance();
    giBox->setGeometry(geoBox);
    giBox->setMaterialCount(1);
    giBox->setMaterial(0, (m_guiMaterialParameters[1].useCutoutTexture) ? m_cutoutMaterial : m_opaqueMaterial);
    giBox["parMaterialIndex"]->setInt(1); // This one has cutout opacity.

    optix::Acceleration accBox = m_context->createAcceleration(m_builder);
    setAccelerationProperties(accBox);

    optix::GeometryGroup ggBox = m_context->createGeometryGroup();
    ggBox->setAcceleration(accBox);
    ggBox->setChildCount(1);
    ggBox->setChild(0, giBox);

    float trafoBox[16] =
    {
      1.0f, 0.0f, 0.0f, -3.0f, // Move to the left.
      0.0f, 1.0f, 0.0f, 1.25f, // The box is modeled with unit coordinates in the range [-1, 1], Move it above the floor plane.
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };
    optix::Matrix4x4 matrixBox(trafoBox);

    optix::Transform trBox = m_context->createTransform();
    trBox->setChild(ggBox);
    trBox->setMatrix(false, matrixBox.getData(), matrixBox.inverse().getData());

    count = m_rootGroup->getChildCount();
    m_rootGroup->setChildCount(count + 1);
    m_rootGroup->setChild(count, trBox);

    // Add a tessellated sphere with 180 longitudes and 90 latitudes, radius 1.0f and fully closed at the upper pole.
    optix::Geometry geoSphere = createSphere(180, 90, 1.0f, M_PIf);

    optix::GeometryInstance giSphere = m_context->createGeometryInstance();
    giSphere->setGeometry(geoSphere);
    giSphere->setMaterialCount(1);
    giSphere->setMaterial(0, (m_guiMaterialParameters[2].useCutoutTexture) ? m_cutoutMaterial : m_opaqueMaterial);
    giSphere["parMaterialIndex"]->setInt(2); // Water material.

    optix::Acceleration accSphere = m_context->createAcceleration(m_builder);
    setAccelerationProperties(accSphere);

    optix::GeometryGroup ggSphere = m_context->createGeometryGroup();
    ggSphere->setAcceleration(accSphere);
    ggSphere->setChildCount(1);
    ggSphere->setChild(0, giSphere);

    optix::Transform trSphere = m_context->createTransform();
    trSphere->setChild(ggSphere);

#if 0 // Motion blur disabled to show the denoiser better on caustics through the water sphere.
    // Implement linear motion blur via the Transform on the sphere.
    float keysLinear[2 * 12] =
    {
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 1.25f,
      0.0f, 0.0f, 1.0f, 0.0f,

      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 1.25f,
      0.0f, 0.0f, 1.0f, 2.5f  // Move 2.5f units in positive z-axis direction.
    };

    trSphere->setMotionKeys(2, RT_MOTIONKEYTYPE_MATRIX_FLOAT12, keysLinear);
    trSphere->setMotionRange(0.0f, 1.0f); // Defaults.
#else
    float trafoSphere[16] =
    {
      1.0f, 0.0f, 0.0f, 0.0f,  // In the center, to the right of the box.
      0.0f, 1.0f, 0.0f, 1.25f, // The sphere is modeled with radius 1.0f. Move it above the floor plane.
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };
    optix::Matrix4x4 matrixSphere(trafoSphere);

    trSphere->setMatrix(false, matrixSphere.getData(), matrixSphere.inverse().getData());
#endif

    count = m_rootGroup->getChildCount();
    m_rootGroup->setChildCount(count + 1);
    m_rootGroup->setChild(count, trSphere);

    // Add a torus.
    optix::Geometry geoTorus = createTorus(180, 180, 0.75f, 0.25f);

    optix::GeometryInstance giTorus = m_context->createGeometryInstance();
    giTorus->setGeometry(geoTorus);
    giTorus->setMaterialCount(1);
    giTorus->setMaterial(0, (m_guiMaterialParameters[3].useCutoutTexture) ? m_cutoutMaterial : m_opaqueMaterial);
    giTorus["parMaterialIndex"]->setInt(3); // Using parameters in sysMaterialParameters[4].

    optix::Acceleration accTorus = m_context->createAcceleration(m_builder);
    setAccelerationProperties(accTorus);

    optix::GeometryGroup ggTorus = m_context->createGeometryGroup();
    ggTorus->setAcceleration(accTorus);
    ggTorus->setChildCount(1);
    ggTorus->setChild(0, giTorus);

    optix::Transform trTorus = m_context->createTransform();
    trTorus->setChild(ggTorus);

#if 1 // Keep motion blur active on the torus to show it together with the denoiser.

    // Implement Scale-Rotation-Translation (SRT) motion blur on the torus.
    // Move to the right, along the positive x-axis and roll around that axis by 180 degrees.

    // Rotation angle is in degrees in the optixQuaternion class.
    const optix::float3 axis = optix::make_float3(1.0f, 0.0f, 0.0f);
    const optix::Quaternion quat0(axis,   0.0f);
    const optix::Quaternion quat1(axis, 180.0f);

    float keysSRT[2 * 16] =
    {
      // Refer to the OptiX Programming Guide which explains what these 16 values per motion key are doing.
      // All Geometries in this demo are modeled around the origin in object coordinates, which is the pivot point (px, py, pz) for the rotation.
      //sx,   a,    b,    px,   sy,   c,    py,   sz,   pz,   qx,          qy,          qz,          qw,          tx,   ty,    tz
        1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, quat0.m_q.x, quat0.m_q.y, quat0.m_q.z, quat0.m_q.w, 2.5f, 1.25f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, quat1.m_q.x, quat1.m_q.y, quat1.m_q.z, quat1.m_q.w, 5.0f, 1.25f, 0.0f
    };

    trTorus->setMotionKeys(2, RT_MOTIONKEYTYPE_SRT_FLOAT16, keysSRT);
    trTorus->setMotionRange(0.0f, 1.0f); // Defaults.
#else
    float trafoTorus[16] =
    {
      1.0f, 0.0f, 0.0f, 2.5f,  // Move it to the right of the sphere.
      0.0f, 1.0f, 0.0f, 1.25f, // The torus has an outer radius of 0.5f. Move it above the floor plane.
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    optix::Matrix4x4 matrixTorus(trafoTorus);

    trTorus->setMatrix(false, matrixTorus.getData(), matrixTorus.inverse().getData());
#endif

    count = m_rootGroup->getChildCount();
    m_rootGroup->setChildCount(count + 1);
    m_rootGroup->setChild(count, trTorus);

    createLights(); // Put lights into the scene.
  }
  catch(optix::Exception& e)
  {
    std::cerr << e.getErrorString() << std::endl;
  }
}


void Application::setAccelerationProperties(optix::Acceleration acceleration)
{
  // To speed up the acceleration structure build for triangles, skip calls to the bounding box program and
  // invoke the special splitting BVH builder for indexed triangles by setting the necessary acceleration properties.
  // Using the fast Trbvh builder which does splitting has a positive effect on the rendering performanc as well!
  if (m_builder == std::string("Trbvh") || m_builder == std::string("Sbvh"))
  {
    // This requires that the position is the first element and it must be float x,y,z.
    acceleration->setProperty("vertex_buffer_name", "attributesBuffer");
    MY_ASSERT(sizeof(VertexAttributes) == 48) ;
    acceleration->setProperty("vertex_buffer_stride", "48");

    acceleration->setProperty("index_buffer_name", "indicesBuffer");
    MY_ASSERT(sizeof(optix::uint3) == 12) ;
    acceleration->setProperty("index_buffer_stride", "12");
  }
}


void Application::createLights()
{
  LightDefinition light;

  // Unused in environment lights.
  light.position = optix::make_float3(0.0f, 0.0f, 0.0f);
  light.vecU     = optix::make_float3(1.0f, 0.0f, 0.0f);
  light.vecV     = optix::make_float3(0.0f, 1.0f, 0.0f);
  light.normal   = optix::make_float3(0.0f, 0.0f, 1.0f);
  light.area     = 1.0f;
  light.emission = optix::make_float3(1.0f, 1.0f, 1.0f);
  // Fields with bindless texture and buffer IDs and the integral for a spherical environment map.
  light.idEnvironmentTexture = RT_TEXTURE_ID_NULL;
  light.environmentIntegral  = 1.0f;
  light.idEnvironmentCDF_U   = RT_BUFFER_ID_NULL;
  light.idEnvironmentCDF_V   = RT_BUFFER_ID_NULL;

  // The environment light is expected in sysLightDefinitions[0]!
  // All other lights are indexed by their position inside the array.
  switch (m_missID)
  {
  case 0: // No environment light at all. Faster than a zero emission constant environment!
  default:
    break;

  case 1: // Constant environment light.
    light.type = LIGHT_ENVIRONMENT;
    light.area = 4.0f * M_PIf; // Unused.

    m_lightDefinitions.push_back(light);
    break;

  case 2: // HDR Environment mapping with loaded texture.
    {
      Picture* picture = new Picture; // Separating image file handling from OptiX texture handling.
      picture->load(m_environmentFilename);

      m_environmentTexture.createEnvironment(picture);

      delete picture;

      // Generate the CDFs for direct environment lighting and the environment texture sampler itself.
      m_environmentTexture.calculateCDF(m_context);
    }

    light.type = LIGHT_ENVIRONMENT;
    light.area = 4.0f * M_PIf; // Unused.

    // Set the bindless texture and buffer IDs inside the LightDefinition.
    light.idEnvironmentTexture = m_environmentTexture.getId();
    light.idEnvironmentCDF_U   = m_environmentTexture.getBufferCDF_U()->getId();
    light.idEnvironmentCDF_V   = m_environmentTexture.getBufferCDF_V()->getId();
    light.environmentIntegral  = m_environmentTexture.getIntegral(); // DAR PERF Could bake the factor 2.0f * M_PIf * M_PIf into the sysEnvironmentIntegral here.

    m_lightDefinitions.push_back(light);
    break;
  }

  if (m_light)  // Add a square area light over the scene objects.
  {
    light.type      = LIGHT_PARALLELOGRAM;                    // A geometric area light with diffuse emission distribution function.
    light.position  = optix::make_float3(-0.5f, 4.0f, -0.5f); // Corner position.
    light.vecU      = optix::make_float3(1.0f, 0.0f, 0.0f);   // To the right.
    light.vecV      = optix::make_float3(0.0f, 0.0f, 1.0f);   // To the front.
    optix::float3 n = optix::cross(light.vecU, light.vecV);   // Length of the cross product is the area.
    light.area     = optix::length(n);                        // Calculate the world space area of that rectangle, unit is [m^2]
    light.normal   = n / light.area;                          // Normalized normal
    light.emission = optix::make_float3(100.0f);              // Radiant exitance in Watt/m^2.

    int lightIndex = int(m_lightDefinitions.size()); // This becomes this light's parLightIndex value.
    m_lightDefinitions.push_back(light);

    // Create the actual area light geometry in the scene. This creates just two triangles, because I do not want to have another intersection routine for code size and performance reasons.
    optix::Geometry geoLight = createParallelogram(light.position, light.vecU, light.vecV, light.normal);

    optix::GeometryInstance giLight = m_context->createGeometryInstance(); // This connects Geometries with Materials.
    giLight->setGeometry(geoLight);
    giLight->setMaterialCount(1);
    giLight->setMaterial(0, m_lightMaterial);
    giLight["parLightIndex"]->setInt(lightIndex);

    optix::Acceleration accLight = m_context->createAcceleration(m_builder);
    setAccelerationProperties(accLight);

    optix::GeometryGroup ggLight = m_context->createGeometryGroup(); // This connects GeometryInstances with Acceleration structures. (All OptiX nodes with "Group" in the name hold an Acceleration.)
    ggLight->setAcceleration(accLight);
    ggLight->setChildCount(1);
    ggLight->setChild(0, giLight);

    // Area lights are defined in world space just to make sampling simpler in this demo.
    // Attach it directly to the scene's root node directly.
    unsigned int count = m_rootGroup->getChildCount();
    m_rootGroup->setChildCount(count + 1);
    m_rootGroup->setChild(count, ggLight);
  }

  // Put the light definitions into the sysLightDefinitions buffer.
  MY_ASSERT((sizeof(LightDefinition) & 15) == 0); // Check alignment to float4

  m_bufferLightDefinitions = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_USER);
  m_bufferLightDefinitions->setElementSize(sizeof(LightDefinition));
  m_bufferLightDefinitions->setSize(m_lightDefinitions.size()); // This can be zero.

  if (m_lightDefinitions.size())
  {
    // Put the light definitions into the sysLightDefinitions buffer.
    void* dst = static_cast<LightDefinition*>(m_bufferLightDefinitions->map(0, RT_BUFFER_MAP_WRITE_DISCARD));
    memcpy(dst, m_lightDefinitions.data(), sizeof(LightDefinition) * m_lightDefinitions.size());
    m_bufferLightDefinitions->unmap();
  }

  m_context["sysLightDefinitions"]->setBuffer(m_bufferLightDefinitions);
  m_context["sysNumLights"]->setInt(int(m_lightDefinitions.size())); // PERF Used often and faster to read than sysLightDefinitions.size().
}
