// Copyright (c) 2013 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <cassert>
#include <cstdio>
#include <cstring>

#include <elf.h>
#include <fcntl.h>
#include <linux/auxvec.h>
#include <unistd.h>

#include "cpuinfo_impl.hpp"

namespace cpuid
{
inline namespace STEINWURF_CPUID_VERSION
{
/// @todo docs
void init_cpuinfo(cpuinfo::impl& info)
{
#if defined(__aarch64__)
    // The Advanced SIMD (NEON) instruction set is required on AArch64
    // (64-bit ARM). Note that /proc/cpuinfo will display "asimd" instead of
    // "neon" in the Features list on a 64-bit ARM CPU.

    auto cpufile = open("/proc/self/auxv", O_RDONLY);
    assert(cpufile);

    Elf64_auxv_t auxv;

    if (cpufile >= 0)
    {
        const auto size_auxv_t = sizeof(Elf64_auxv_t);
        while (read(cpufile, &auxv, size_auxv_t) == size_auxv_t)
        {
            if (auxv.a_type == AT_HWCAP)
            {
                info.m_has_vfpu = info.m_has_vfpv3 = info.m_has_vfpv4 = (auxv.a_un.a_val & (1 << 0)) != 0;
                info.m_has_neon = (auxv.a_un.a_val & (1 << 1)) != 0;
                info.m_has_aes = (auxv.a_un.a_val & (1 << 3)) != 0;
                break;
            }
        }

        close(cpufile);
    }
    else
    {
        info.m_has_aes = false;
        info.m_has_neon = false;
        info.m_has_vfpu = false;
        info.m_has_vfpv3 = false;
        info.m_has_vfpv4 = false;
    }

#else
    // Runtime detection of NEON is necessary on 32-bit ARM CPUs
    //
    // Follow recommendation from Cortex-A Series Programmer's guide
    // in Section 20.1.7 Detecting NEON. The guide is available at
    // Steinwurf's Google drive: steinwurf/technical/experimental/cpuid

    auto cpufile = open("/proc/self/auxv", O_RDONLY);
    assert(cpufile);

    Elf32_auxv_t auxv;

    if (cpufile >= 0)
    {
        const auto size_auxv_t = sizeof(Elf32_auxv_t);
        while (read(cpufile, &auxv, size_auxv_t) == size_auxv_t)
        {
            if (auxv.a_type == AT_HWCAP)
            {
                info.m_has_neon = (auxv.a_un.a_val & (1 << 12)) != 0;
                info.m_has_vfpu = (auxv.a_un.a_val & (1 << 6)) != 0;
                info.m_has_vfpv3 = (auxv.a_un.a_val & (1 << 13)) != 0;
                info.m_has_vfpv4 = (auxv.a_un.a_val & (1 << 16)) != 0;
                break;
            }
            if (auxv.a_type == AT_HWCAP2)
            {
                info.m_has_aes = (auxv.a_un.a_val & (1 << 0)) != 0;
                break;
            }
        }

        close(cpufile);
    }
    else
    {
        info.m_has_neon = false;
    }
#endif
}
}
}
