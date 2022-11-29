// Copyright (c) 2013 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include "cpuinfo.hpp"

namespace cpuid
{
inline namespace STEINWURF_CPUID_VERSION
{

struct cpuinfo::impl
{
    impl() :
        m_has_fpu(false), m_has_mmx(false), m_has_mmx_ext(false),
        m_has_sse(false), m_has_sse2(false), m_has_sse3(false),
        m_has_ssse3(false), m_has_sse4_1(false), m_has_sse4_2(false),
        m_has_pclmulqdq(false), m_has_avx(false), m_has_avx2(false),
        m_has_f16c(false), m_has_vmx(false), m_has_aes(false),
        m_has_popcnt(false), m_has_movbe(false), m_has_cmov(false),
        m_has_neon(false), m_has_vfpu(false), m_has_ps(false),
        m_has_vfpv3(false), m_has_vfpv4(false)
    {
    }

    bool m_has_fpu;
    bool m_has_mmx;
    bool m_has_mmx_ext;
    bool m_has_sse;
    bool m_has_sse2;
    bool m_has_sse3;
    bool m_has_ssse3;
    bool m_has_sse4_1;
    bool m_has_sse4_2;
    bool m_has_pclmulqdq;
    bool m_has_avx;
    bool m_has_avx2;
    bool m_has_f16c;
    bool m_has_vmx;
    bool m_has_aes;
    bool m_has_popcnt;
    bool m_has_movbe;
    bool m_has_cmov;
    bool m_has_neon;
    bool m_has_vfpu;
    bool m_has_ps;
    bool m_has_vfpv3;
    bool m_has_vfpv4;
};
}
}
