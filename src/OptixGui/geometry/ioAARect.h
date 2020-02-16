#ifndef IO_AA_RECT_H
#define IO_AA_RECT_H

#include "ioGeometry.h"

#include <optix.h>
#include <optixu/optixpp.h>

extern "C" const char aarect_ptx_c[];

enum Axis { X_AXIS, Y_AXIS, Z_AXIS };

class ioAARect : public ioGeometry
{
public:
    ioAARect() {}

    ioAARect(const float a0, const float a1, const float b0, const float b1, const float k, const bool flip, const Axis orientation)
        {
            m_a0 = a0;
            m_a1 = a1;
            m_b0 = b0;
            m_b1 = b1;
            m_k  = k;
            m_flip = flip;
            kind = orientation;
        }

    void init(optix::Context& context) {
        m_geo = context->createGeometry();
        m_geo->setPrimitiveCount(1);

        if (kind == X_AXIS) {
            m_geo->setBoundingBoxProgram(context->createProgramFromPTXString(aarect_ptx_c, "getBoundsX"));
            m_geo->setIntersectionProgram(context->createProgramFromPTXString(aarect_ptx_c, "hitRectX"));
        } else if  (kind == Y_AXIS) {
            m_geo->setBoundingBoxProgram(context->createProgramFromPTXString(aarect_ptx_c, "getBoundsY"));
            m_geo->setIntersectionProgram(context->createProgramFromPTXString(aarect_ptx_c, "hitRectY"));
        } else if  (kind == Z_AXIS) {
            m_geo->setBoundingBoxProgram(context->createProgramFromPTXString(aarect_ptx_c, "getBoundsZ"));
            m_geo->setIntersectionProgram(context->createProgramFromPTXString(aarect_ptx_c, "hitRectZ"));
        } else { // should never reach this branch
            m_geo->setBoundingBoxProgram(context->createProgramFromPTXString(aarect_ptx_c, "getBoundsY"));
            m_geo->setIntersectionProgram(context->createProgramFromPTXString(aarect_ptx_c, "hitRectY"));
        }
        _init();
    }

    void _init() {
        m_geo["a0"]->setFloat(m_a0);
        m_geo["a1"]->setFloat(m_a1);
        m_geo["b0"]->setFloat(m_b0);
        m_geo["b1"]->setFloat(m_b1);
        m_geo["k"]->setFloat(m_k);
        m_geo["flip"]->setInt(m_flip);
    }



private:
    float m_a0;
    float m_a1;
    float m_b0;
    float m_b1;
    float m_k;
    bool  m_flip;
    Axis kind;
};

#endif //!IO_AA_RECT_H
