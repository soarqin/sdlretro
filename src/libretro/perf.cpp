#include "perf.h"

#include "util.h"
#include "libretro.h"

#include <cpuinfo.hpp>
#include <spdlog/spdlog.h>

namespace {
/* Returns current time in microseconds.
 * Tries to use the most accurate timer available.
 */
extern "C" retro_time_t retro_perf_get_time_usec() {
    return util::get_ticks_usec();
}

/* A simple counter. Usually nanoseconds, but can also be CPU cycles.
 * Can be used directly if desired (when creating a more sophisticated
 * performance counter system).
 * */
extern "C" retro_perf_tick_t retro_perf_get_counter() {
    return util::get_ticks_perfcounter();
}

/* Returns a bit-mask of detected CPU features (RETRO_SIMD_*). */
extern "C" uint64_t retro_get_cpu_features() {
    static cpuid::cpuinfo info;
    uint64_t result = 0;

    if (info.has_sse()) { result |= RETRO_SIMD_SSE; }
    if (info.has_sse2()) { result |= RETRO_SIMD_SSE2; }
    if (info.has_vmx()) { result |= RETRO_SIMD_VMX; }
    if (info.has_avx()) { result |= RETRO_SIMD_AVX; }
    if (info.has_neon()) {
        result |= RETRO_SIMD_NEON;
#if defined(__aarch64__)
        result |= RETRO_SIMD_ASIMD;
#endif
    }
    if (info.has_sse3()) { result |= RETRO_SIMD_SSE3; }
    if (info.has_ssse3()) { result |= RETRO_SIMD_SSSE3; }
    if (info.has_mmx()) { result |= RETRO_SIMD_MMX; }
    if (info.has_mmx_ext()) { result |= RETRO_SIMD_MMXEXT; }
    if (info.has_sse4_1()) { result |= RETRO_SIMD_SSE4; }
    if (info.has_sse4_2()) { result |= RETRO_SIMD_SSE42; }
    if (info.has_avx2()) { result |= RETRO_SIMD_AVX2; }
    if (info.has_vfpu()) { result |= RETRO_SIMD_VFPU; }
    if (info.has_aes()) { result |= RETRO_SIMD_AES; }
    if (info.has_vfpv3()) { result |= RETRO_SIMD_VFPV3; }
    if (info.has_vfpv4()) { result |= RETRO_SIMD_VFPV4; }
    if (info.has_popcnt()) { result |= RETRO_SIMD_POPCNT; }
    if (info.has_movbe()) { result |= RETRO_SIMD_MOVBE; }
    if (info.has_cmov()) { result |= RETRO_SIMD_CMOV; }
#if defined(__ALTIVEC__)
    result |= RETRO_SIMD_VMX;
#elif defined(XBOX360)
    result |= RETRO_SIMD_VMX128;
#elif defined(PSP) || defined(PS2)
    result |= RETRO_SIMD_VFPU;
#elif defined(GEKKO)
    result |= RETRO_SIMD_PS;
#endif
    return result;
}

enum {
    MAX_COUNTERS = 64
};

int curr_counters = 0;
retro_perf_counter *counters[MAX_COUNTERS];

/* Asks frontend to log and/or display the state of performance counters.
 * Performance counters can always be poked into manually as well.
 */
extern "C" void retro_perf_log() {
    for (int i = 0; i < curr_counters; ++i) {
        spdlog::trace("Performance counter {}: {} / {}", counters[i]->ident, counters[i]->total / counters[i]->call_cnt, counters[i]->total);
    }
}

/* Register a performance counter.
 * ident field must be set with a discrete value and other values in
 * retro_perf_counter must be 0.
 * Registering can be called multiple times. To avoid calling to
 * frontend redundantly, you can check registered field first. */
extern "C" void retro_perf_register(struct retro_perf_counter *counter) {
    if (curr_counters >= MAX_COUNTERS) {
        return;
    }
    counters[curr_counters++] = counter;
    counter->registered = true;
}

/* Starts a registered counter. */
extern "C" void retro_perf_start(struct retro_perf_counter *counter) {
    counter->start = util::get_ticks_perfcounter();
}

/* Stops a registered counter. */
extern "C" void retro_perf_stop(struct retro_perf_counter *counter) {
    counter->call_cnt++;
    counter->total += util::get_ticks_perfcounter() - counter->start;
}

}

void libretro_get_perf_callback(struct retro_perf_callback *cb) {
    cb->get_time_usec = retro_perf_get_time_usec;
    cb->get_cpu_features = retro_get_cpu_features;

    cb->get_perf_counter = retro_perf_get_counter;
    cb->perf_register = retro_perf_register;
    cb->perf_start = retro_perf_start;
    cb->perf_stop = retro_perf_stop;
    cb->perf_log = retro_perf_log;
}
