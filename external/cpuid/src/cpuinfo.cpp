// Copyright (c) 2013 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <platform/config.hpp>

#include "cpuinfo.hpp"
#include "detail/cpuinfo_impl.hpp"

#if defined(PLATFORM_GCC_COMPATIBLE_X86)
    #include "detail/init_gcc_x86.hpp"
#elif defined(PLATFORM_MSVC_X86) && !defined(PLATFORM_WINDOWS_PHONE)
    #include "detail/init_msvc_x86.hpp"
#elif defined(PLATFORM_MSVC_ARM)
    #include "detail/init_msvc_arm.hpp"
#elif defined(PLATFORM_CLANG_ARM) && defined(PLATFORM_IOS)
    #include "detail/init_ios_clang_arm.hpp"
#elif defined(PLATFORM_GCC_COMPATIBLE_ARM) && defined(PLATFORM_LINUX)
    #include "detail/init_linux_gcc_arm.hpp"
#else
    #include "detail/init_unknown.hpp"
#endif

namespace cpuid
{

inline namespace STEINWURF_CPUID_VERSION
{

cpuinfo::cpuinfo() :
    m_impl(new impl)
{
    init_cpuinfo(*m_impl);
}

cpuinfo::~cpuinfo()
{
}

// x86 member functions
bool cpuinfo::has_fpu() const
{
    return m_impl->m_has_fpu;
}

bool cpuinfo::has_mmx() const
{
    return m_impl->m_has_mmx;
}

bool cpuinfo::has_mmx_ext() const
{
    return m_impl->m_has_mmx_ext;
}

bool cpuinfo::has_sse() const
{
    return m_impl->m_has_sse;
}

bool cpuinfo::has_sse2() const
{
    return m_impl->m_has_sse2;
}

bool cpuinfo::has_sse3() const
{
    return m_impl->m_has_sse3;
}

bool cpuinfo::has_ssse3() const
{
    return m_impl->m_has_ssse3;
}

bool cpuinfo::has_sse4_1() const
{
    return m_impl->m_has_sse4_1;
}

bool cpuinfo::has_sse4_2() const
{
    return m_impl->m_has_sse4_2;
}

bool cpuinfo::has_pclmulqdq() const
{
    return m_impl->m_has_pclmulqdq;
}

bool cpuinfo::has_avx() const
{
    return m_impl->m_has_avx;
}

bool cpuinfo::has_avx2() const
{
    return m_impl->m_has_avx2;
}

bool cpuinfo::has_f16c() const
{
    return m_impl->m_has_f16c;
}

bool cpuinfo::has_vmx() const
{
    return m_impl->m_has_vmx;
}

bool cpuinfo::has_aes() const
{
    return m_impl->m_has_aes;
}

bool cpuinfo::has_popcnt() const
{
    return m_impl->m_has_popcnt;
}

bool cpuinfo::has_movbe() const
{
    return m_impl->m_has_movbe;
}

bool cpuinfo::has_cmov() const
{
    return m_impl->m_has_cmov;
}

// ARM member functions
bool cpuinfo::has_neon() const
{
    return m_impl->m_has_neon;
}

bool cpuinfo::has_vfpu() const
{
    return m_impl->m_has_vfpu;
}

bool cpuinfo::has_ps() const
{
    return m_impl->m_has_ps;
}

bool cpuinfo::has_vfpv3() const
{
    return m_impl->m_has_vfpv3;
}

bool cpuinfo::has_vfpv4() const
{
    return m_impl->m_has_vfpv4;
}

}
}
