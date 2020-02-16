#include <iostream>

#include <optix.h>
#include <optixu/optixpp.h>

#include "Director.h"

void Director::init(unsigned int width, unsigned int height,
                    unsigned int samples)
{
    m_Nx = width;
    m_Ny = height;
    m_Ns = samples;
    m_maxRayDepth = 50;

    initContext();

    initOutputBuffer();
    m_context["sysOutputBuffer"]->set(m_outputBuffer);
}

void Director::destroy()
{
    m_scene.destroy();
    m_outputBuffer->destroy();
    m_context->destroy();
}

void Director::initContext()
{

    int RTX = 1;
    unsigned int DRV_MAJOR = 0;
    unsigned int DRV_MINOR = 0;

    RTresult res;
    if (_verbose) {
        res = rtGlobalGetAttribute(RT_GLOBAL_ATTRIBUTE_DISPLAY_DRIVER_VERSION_MAJOR,
                                   sizeof(DRV_MAJOR), &(DRV_MAJOR));
        res = rtGlobalGetAttribute(RT_GLOBAL_ATTRIBUTE_DISPLAY_DRIVER_VERSION_MINOR,
                                   sizeof(DRV_MINOR), &(DRV_MINOR));
        std::cerr << "INFO: Display driver version: " << DRV_MAJOR << '.' << DRV_MINOR << std::endl;
    }

    res = rtGlobalSetAttribute(RT_GLOBAL_ATTRIBUTE_ENABLE_RTX, sizeof(RTX),
                               &(RTX));
    if (res != RT_SUCCESS) {
        std::cerr << "Error: RTX execution strategy is required. Exiting." << std::endl;
        exit(0);
    } else if (_verbose) {
        std::cerr <<"INFO: OptiX RTX execution mode is ON." << std::endl;
    }

    m_context = optix::Context::create();

    // Should be handled per scene?
    m_context->setRayTypeCount(1);

    // Set Output Debugging via rtPrintf
    if (_debug) {
        m_context->setPrintEnabled(1);
        // default: 32676? // m_context->setPrintBufferSize(4096);
    }

}

void Director::initOutputBuffer()
{
    m_outputBuffer = m_context->createBuffer(RT_BUFFER_OUTPUT);
    m_outputBuffer->setFormat(RT_FORMAT_FLOAT3);
    m_outputBuffer->setSize(m_Nx, m_Ny);
}

void Director::createScene(unsigned int sceneNumber)
{

    int error = m_scene.init(m_context, m_Nx, m_Ny, m_Ns, m_maxRayDepth, sceneNumber);
    if (error) {
        int exit_code = EXIT_FAILURE;
        std::exit( exit_code );
    }

    if (_verbose) {
        std::string desc = m_scene.getDescription();
        std::cerr << "INFO: Scene description: " << desc << std::endl;
    }
}

void Director::renderFrame()
{
    m_context->validate();
    m_context->launch(0,         // Program ID
                      m_Nx, m_Ny // Launch Dimensions
        );
}

void Director::printPPM()
{
    void* bufferData = m_outputBuffer->map();
    //   Print ppm header
    std::cout << "P3\n" << m_Nx << " " << m_Ny << "\n255\n";
    //   Parse through bufferdata
    for (int j = m_Ny - 1; j >= 0;  j--)
    {
        for (int i = 0; i < m_Nx; i++)
        {
            float* floatData = ((float*)bufferData) + (3*(m_Nx*j + i));
            float r = floatData[0];
            float g = floatData[1];
            float b = floatData[2];
            int ir = int(255.99f * r);
            int ig = int(255.99f * g);
            int ib = int(255.99f * b);
            std::cout << ir << " " << ig << " " << ib << "\n";
        }
    }
    m_outputBuffer->unmap();
}
