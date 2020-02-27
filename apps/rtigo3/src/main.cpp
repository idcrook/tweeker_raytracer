/*
 * Copyright (c) 2013-2020, NVIDIA CORPORATION. All rights reserved.
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

#include "shaders/config.h"

#include "inc/Application.h"

#include <IL/il.h>

#include <algorithm>
#include <iostream>


static Application* g_app = nullptr;

static void callbackError(int error, const char* description)
{
  std::cerr << "Error: "<< error << ": " << description << std::endl;
}


static int runApp(Options const& options)
{
  int width  = std::max(1, options.getWidth());
  int height = std::max(1, options.getHeight());

  // GLFWwindow* window = glfwCreateWindow(width, height, "rtigo3 - Copyright (c) 2020 NVIDIA Corporation", NULL, NULL);
  // if (!window)
  // {
  //   callbackError(APP_ERROR_CREATE_WINDOW, "glfwCreateWindow() failed.");
  //   glfwTerminate();
  //   return APP_ERROR_CREATE_WINDOW;
  // }

  // glfwMakeContextCurrent(window);

  // if (glewInit() != GL_NO_ERROR)
  // {
  //   callbackError(APP_ERROR_GLEW_INIT, "GLEW failed to initialize.");
  //   glfwTerminate();
  //   return APP_ERROR_GLEW_INIT;
  // }

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
  // GL 3.0 - empirically determined on ubuntu 19.10
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
#endif
  // glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  // glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
  GLFWwindow* window = glfwCreateWindow(width, height, "rtigo3 - Copyright (c) 2020 NVIDIA Corporation", NULL, NULL);
  if (!window)
  {
    callbackError(APP_ERROR_CREATE_WINDOW, "glfwCreateWindow() failed.");
    glfwTerminate();
    return APP_ERROR_CREATE_WINDOW;
  }

  glfwMakeContextCurrent(window);
  //glfwSwapInterval(1); // Enable vsync

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

  if (err != GL_NO_ERROR)
  {
    callbackError(APP_ERROR_OPENGL_LOADER_INIT, "OpenGL Loader failed to initialize.");
    glfwTerminate();
    return APP_ERROR_OPENGL_LOADER_INIT;
  }
  else
  {
    std::cerr << "INFO: OpenGL renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cerr << "INFO: OpenGL version in GLFW window context: "
              << glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR)
              << "."
              << glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR)
              << std::endl;
    std::cerr << "INFO: GLFW version: " << glfwGetVersionString() << std::endl;
  }


  ilInit(); // Initialize DevIL once.

  g_app = new Application(window, options);

  if (!g_app->isValid())
  {
    std::cerr << "ERROR: Application() failed to initialize successfully." << std::endl;
    ilShutDown();
    glfwTerminate();
    return APP_ERROR_APP_INIT;
  }

  const int mode = std::max(0, options.getMode());

  if (mode == 0) // Interactive, default.
  {
    // Main loop
    bool finish = false;
    while (!finish && !glfwWindowShouldClose(window))
    {
      glfwPollEvents(); // Render continuously. Battery drainer!

      glfwGetFramebufferSize(window, &width, &height);

      g_app->reshape(width, height);
      g_app->guiNewFrame();
      //g_app->guiReferenceManual();  // HACK The ImGUI "Programming Manual" as example code.
      g_app->guiWindow();             // This application's GUI window rendering commands.
      g_app->guiEventHandler();       // SPACE to toggle the GUI windows and all mouse tracking via GuiState.
      finish = g_app->render();       // OptiX rendering, returns true when benchmark is enabled and the samples per pixel have been rendered.
      g_app->display();               // OpenGL display always required to lay the background for the GUI.
      g_app->guiRender();             // Render all ImGUI elements at last.

      glfwSwapBuffers(window);

      //glfwWaitEvents(); // Render only when an event is happening. Needs some glfwPostEmptyEvent() to prevent GUI lagging one frame behind when ending an action.
    }
  }
  else if (mode == 1) // Batched benchmark single shot. // FIXME When not using anything OpenGL, the whole window and OpenGL setup could be removed.
  {
    g_app->benchmark();
  }

  delete g_app;

  ilShutDown();

  return APP_EXIT_SUCCESS;
}


int main(int argc, char *argv[])
{
  glfwSetErrorCallback(callbackError);

  if (!glfwInit())
  {
    callbackError(APP_ERROR_GLFW_INIT, "GLFW failed to initialize.");
    return APP_ERROR_GLFW_INIT;
  }

  int result = APP_ERROR_UNKNOWN;

  Options options;

  if (options.parseCommandLine(argc, argv))
  {
    result = runApp(options);
  }

  return result;
}
