#pragma once

// #define RELEASE_ASSERTS
// #define JCC_PROFILE_BUILD

#ifdef RELEASE_ASSERTS
#undef NDEBUG
#endif

#ifdef JCC_PROFILE_BUILD
#include "small_profiler.h"
#define JCC_PROFILE() PROFILE();
#define JCC_PROFILE_SCOPE(x) PROFILE_SCOPE(x)
#else
#define JCC_PROFILE()
#define JCC_PROFILE_SCOPE(x)
#endif

#include "cprint.h"

#include <vector>
#include <string>
#include <cassert>

using namespace cprint;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

// ------------------------------------
// TODO this is all duplicated from JAB
// should be changed in the future
// ------------------------------------
enum class Arch : i8 { unknown, x64, aarch64 };

enum class OS : i8 {
    unknown,
    windows,
    linux,
    macos,
    freebsd,
    android,
};

constexpr OS get_host_os() {
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#define JCC_OS_WINDOWS
    return OS::windows;
#elif defined(__ANDROID__)
#define JCC_OS_ANDROID
    return OS::android;
#elif defined(__FreeBSD__)
#define JCC_OS_FREEBSD
    return OS::freebsd;
#elif defined(__APPLE__) || defined(__MACH__)
#define JCC_OS_MACOS
    return OS::macos;
#elif defined(__linux__)
#define JCC_OS_LINUX
    return OS::linux;
#elif defined(unix) || defined(__unix) || defined(__unix__)
#define JCC_OS_LINUX
    return OS::linux; // meh
#else
    assert("unsupported host os");
#endif
}

constexpr Arch get_host_arch() {
#if defined(__amd64__) || defined(__x86_64__) || defined(_M_X64) ||            \
    defined(_M_AMD64)
#define JCC_ARCH_X64
    return Arch::x64;
#elif defined(_M_ARM64) || defined(__aarch64__)
#define JCC_ARCH_AARCH64
    return Arch::aarch64;
#else
    assert("unsupported host arch");
#endif
}

// get host os/arch here
constexpr OS host_os = get_host_os();
constexpr Arch host_arch = get_host_arch();
// ------------------------------------
// end of duplicated section
// ----------------------------
