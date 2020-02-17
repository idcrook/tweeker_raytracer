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

#ifndef PER_RAY_DATA_CUH
#define PER_RAY_DATA_CUH

#include "app_config.cuh"

#include "random_number_generators.cuh"

// Set if (0.0f <= wo_dot_ng), means looking onto the front face. (Edge-on is explicitly handled as frontface for the material stack.)
#define FLAG_FRONTFACE      0x00000010

// Highest bit set means terminate path.
#define FLAG_TERMINATE      0x80000000


// Note that the fields are ordered by CUDA alignment.
struct PerRayData
{
  optix::float3 pos;            // Current surface hit point, in world space

  optix::float3 wo;             // Outgoing direction, to observer, in world space.
  optix::float3 wi;             // Incoming direction, to light, in world space.

  optix::float3 radiance;       // Radiance along the current path segment.
  int           flags;          // Bitfield with flags. See FLAG_* defines for its contents.

  optix::float3 f_over_pdf;     // BSDF sample throughput, pre-multiplied f_over_pdf = bsdf.f * fabsf(dot(wi, ns) / bsdf.pdf;
  float         pdf;            // The last BSDF sample's pdf, tracked for multiple importance sampling.

  unsigned int  seed;           // Random number generator input.
};

#endif // PER_RAY_DATA_CUH
