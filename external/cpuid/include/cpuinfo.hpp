// Copyright (c) 2013 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <memory>

#include "version.hpp"

namespace cpuid
{
inline namespace STEINWURF_CPUID_VERSION
{
/// The cpuinfo object extract information about which, if any, additional
/// instructions are supported by the CPU.
class cpuinfo
{
public:
    /// Constructor for feature detection with default values
    cpuinfo();

    /// Destructor
    ~cpuinfo();

    /// Has X87 FPU
    bool has_fpu() const;

    /// Return true if the CPU supports MMX
    bool has_mmx() const;
    bool has_mmx_ext() const;

    /// Return true if the CPU supports SSE
    bool has_sse() const;

    /// Return true if the CPU supports SSE2
    bool has_sse2() const;

    /// Return true if the CPU supports SSE3
    bool has_sse3() const;

    /// Return true if the CPU supports SSSE3
    bool has_ssse3() const;

    /// Return true if the CPU supports SSE 4.1
    bool has_sse4_1() const;

    /// Return true if the CPU supports SSE 4.2
    bool has_sse4_2() const;

    /// Return true if the CPU supports pclmulqdq
    bool has_pclmulqdq() const;

    /// Return true if the CPU supports AVX
    bool has_avx() const;

    /// Return true if the CPU supports AVX2
    bool has_avx2() const;

    /// Return true if the CPU supports F16C
    bool has_f16c() const;

    bool has_vmx() const;
    bool has_aes() const;
    bool has_popcnt() const;
    bool has_movbe() const;
    bool has_cmov() const;

    /// Return true if the CPU supports NEON
    bool has_neon() const;
    bool has_vfpu() const;
    bool has_ps() const;
    bool has_vfpv3() const;
    bool has_vfpv4() const;

public:
    /// Private implementation
    struct impl;

private:
    /// Pimpl pointer
    std::unique_ptr<impl> m_impl;
};
}
}
