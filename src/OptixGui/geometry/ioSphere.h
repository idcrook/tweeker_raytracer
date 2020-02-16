#ifndef IO_SPHERE_H
#define IO_SPHERE_H

#include "ioGeometry.h"

#include <optix.h>
#include <optixu/optixpp.h>

extern "C" const char sphere_ptx_c[];

class ioSphere : public ioGeometry
{
public:

  ioSphere(const float x, const float y, const float z, const float r)
    {
      m_cx = x;
      m_cy = y;
      m_cz = z;
      m_r = r;
    }

  virtual void init(optix::Context& context)
    {
      m_geo = context->createGeometry();
      m_geo->setPrimitiveCount(1);
      m_geo->setBoundingBoxProgram(
        context->createProgramFromPTXString(sphere_ptx_c, "getBounds")
        );
      m_geo->setIntersectionProgram(
        context->createProgramFromPTXString(sphere_ptx_c, "intersection")
        );
      m_geo["center"]->setFloat(m_cx, m_cy, m_cz);
      m_geo["radius"]->setFloat(m_r);

    }

private:
  float m_cx;
  float m_cy;
  float m_cz;
  float m_r;
};

#endif //!IO_SPHERE_H
