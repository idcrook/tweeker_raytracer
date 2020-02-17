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

#pragma once

#ifndef APPLICATION_H
#define APPLICATION_H

#include <optix.h>
#include <optixu/optixpp.h>
// #include <optixu/optixpp_namespace.h>


#include "imgui.h"

#include "bindings/imgui_impl_glfw.h"
// #include "bindings/imgui_impl_opengl2.h"
#include "bindings/imgui_impl_opengl3.h"

#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>  // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>  // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

#include "include/PinholeCamera.h"

#include <string>
#include <map>


// For rtDevice*() function error checking. No OptiX context present at that time.
#define RT_CHECK_ERROR_NO_CONTEXT( func ) \
  do { \
    RTresult code = func; \
    if (code != RT_SUCCESS) \
      std::cerr << "ERROR: Function " << #func << std::endl; \
  } while (0)

enum GuiState
{
  GUI_STATE_NONE,
  GUI_STATE_ORBIT,
  GUI_STATE_PAN,
  GUI_STATE_DOLLY,
  GUI_STATE_FOCUS
};

class Application
{
public:
  Application(GLFWwindow*        window,
              const int          width,
              const int          height,
              const unsigned int devices,
              const unsigned int stackSize,
              const bool         interop);
  ~Application();

  bool isValid() const;

  void reshape(int width, int height);

  bool render();
  void display();

  void screenshot(std::string const& filename);

  void guiNewFrame();
  void guiWindow();
  void guiEventHandler();
  void guiRender();

  void guiReferenceManual(); // DAR HACK DEBUG The IMGUI "programming manual" in form of a live window.

private:
  void getSystemInformation();

  void initOpenGL();
  void checkInfoLog(const char *msg, GLuint object);
  void initGLSL();

  void initOptiX();
  void initRenderer();
  void initPrograms();
  void initScene();

private:
  GLFWwindow* m_window;
  int         m_width;
  int         m_height;
  bool        m_isValid;

  // Application command line parameters.
  unsigned int m_devicesEncoding;
  unsigned int m_stackSize;
  bool         m_interop;

  // OpenGL variables:
  GLuint m_pboOutputBuffer;
  GLuint m_hdrTexture;

  // OptiX variables:
  optix::Context m_context;

  optix::Buffer m_bufferOutput;

  std::map<std::string, optix::Program> m_mapOfPrograms;

  // GLSL shaders objects and program.
  GLuint m_glslVS;
  GLuint m_glslFS;
  GLuint m_glslProgram;

  GuiState m_guiState;

  bool m_isWindowVisible; // Hide the GUI window completely with SPACE key.

  float m_mouseSpeedRatio;

  PinholeCamera m_pinholeCamera;

  // Colors for the miss_gradient program:
  optix::float3 m_colorBottom;
  optix::float3 m_colorTop;

};

#endif // APPLICATION_H
