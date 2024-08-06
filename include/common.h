#pragma once

#define RELEASE_ASSERTS
// #define JCC_PROFILE_BUILD
#define VERBOSE_ICE

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

#ifdef VERBOSE_ICE
#include <stacktrace>

void jcc_ice_assert(const char *expr, std::stacktrace s,
                    const char *message = nullptr);

#define ice(expression, ...)                                                   \
    (void)((!!(expression)) ||                                                 \
           (jcc_ice_assert(#expression, std::stacktrace::current(),            \
                           __VA_ARGS__),                                       \
            0))
#else
#include <cassert>

#define ice(expression, ...) assert(expression)

#endif

#include <vector>
#include <string>
#include <string_view>
#include <unordered_set>

#include "cprint.h"
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
// TODO this is all duplicated from JB
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

struct InputFile {
    std::string filepath;
    std::string text;
};

[[nodiscard]] std::string read_file(const std::string &);

// Interned String
struct Strand {
    Strand(): ptr{nullptr} {}
    Strand(const Strand &view): ptr{view.ptr} {}
    Strand(Strand &view): ptr{view.ptr} {}
    Strand(Strand &&view): ptr{view.ptr} {}

    Strand(const char *);
    Strand(std::string);
    Strand(std::string *str_ptr_temp) {
        ptr = intern(*str_ptr_temp);
    }

    Strand& operator=(const char *);
    Strand& operator=(std::string);
    // TODO remove
    Strand& operator=(std::string *str_ptr_temp) {
        ptr = intern(*str_ptr_temp);
        return *this;
    }

    std::string& value() const {
        return *ptr;
    }

    operator std::string() const {
        return value();
    }

    // template <typename T> Strand(T &&ref): view(std::forward<T>(ref)) {}

    bool has_value() const { return !empty(); }
    bool empty() const { return !ptr || ptr->empty(); }

    friend std::ostream &operator<<(std::ostream &, const Strand &);

    bool operator==(const Strand &other) const {
        return ptr == other.ptr;
    }

    bool operator==(Strand &&other) const {
        return ptr == other.ptr;
    }

    std::string *ptr;
    
    std::string* intern(std::string);
    std::string* intern(const char *);
};

template <>
struct std::hash<Strand> {
    size_t operator()(const Strand& s) const {
        return (uintptr_t)s.ptr;
    }
};

// template <size_t Capacity = 1000,
//           typename Hash = std::hash<std::string_view>,
//           typename PoolAllocator = std::allocator<char>,
//           typename ViewAllocator = std::allocator<std::string_view>>
// class strand_pool {
//     using pointer = typename std::allocator_traits<PoolAllocator>::pointer;

//   public:
//     constexpr strand_pool() : memory_block_start_(nullptr), current_position_(nullptr) {
//         PoolAllocator allocator;
//         memory_block_start_ = allocator.allocate(Capacity);
//         current_position_ = memory_block_start_;
//     }

//     constexpr ~strand_pool() = default;
//     constexpr strand_pool(const strand_pool &) = default;
//     constexpr strand_pool(strand_pool &&) = default;
//     constexpr strand_pool &operator=(const strand_pool &) = default;
//     constexpr strand_pool &operator=(strand_pool &&) = default;

//     [[nodiscard]] constexpr std::string_view create(std::string &str) {
//         // if string is in table
//         pointer location = find(str);
//         if (location != nullptr)
//             return std::string_view{location, str.size()};

//         std::memcpy(current_position_, str.data(), str.size() + 1);
//         auto result = std::string_view{
//             std::exchange(current_position_, current_position_ + str.size()), str.size()};
//         table_.insert(result);
//         return result;
//     }

//     // [[nodiscard]] constexpr std::string_view create(std::string &&str) {
//     //     // if string is in table
//     //     auto v = std::string_view{str};
//     //     auto location = find(v);
//     //     if (location != nullptr)
//     //         return std::string_view{location, str.size()};

//     //     std::memcpy(current_position_, str.data(), str.size() + 1);
//     //     auto result = std::string_view{
//     //         std::exchange(current_position_, current_position_ + str.size()), str.size()};
//     //     table_.insert(result);
//     //     return result;
//     // }
//     // [[nodiscard]] constexpr Strand create(std::string &&str) {
//     //     // if string is in table
//     //     auto v = std::string_view{str};
//     //     auto location = find(v);
//     //     if (!location.empty())
//     //         return location;

//     //     std::memcpy(current_position_, str.data(), str.size() + 1);
//     //     auto result = Strand(std::string_view{
//     //         std::exchange(current_position_, current_position_ + str.size()), str.size()});
//     //     table_.insert(result.view);
//     //     return Strand{result};
//     // }

//     [[nodiscard]] constexpr std::string_view find(std::string_view string) {

//         auto result = table_.find(string);

//         if (result != table_.end())
//             return *result;

//         return std::string_view{};
//     }

//   private:
//     pointer memory_block_start_;
//     pointer current_position_;

//     std::unordered_set<std::string_view, Hash, std::equal_to<>, ViewAllocator> table_;
// };