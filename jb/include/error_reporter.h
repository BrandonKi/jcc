#pragma once

#include <cprint.h>

#include <string_view>
#include <string>
#include <format>
#include <cassert>

namespace jb {

enum Severity { none, info, warn, fatal };

enum ErrorCode { f1000, f2000, f3000 };

std::string_view get_error_msg(ErrorCode code) {
    using namespace std::literals;
    switch (code) {
    case f1000:
        return "Invalid command line argument: {} \n"sv;
    case f2000:
        return ""sv;
    case f3000:
        return ""sv;
    default:
        assert(false);
        return ""sv;
    }
}

inline void print_with_severity(std::string str, Severity severity) {
    using namespace cprint;
    switch (severity) {
    case none:
        println(str);
        break;
    case info:
        println("Info: " + str, BRIGHT_BLUE);
        break;
    case warn:
        println("Warn: " + str, YELLOW);
        break;
    case fatal:
        println("Error: " + str, RED);
        break;
    default:
        assert(false);
    }
}

template <typename... T>
void report_error(Severity severity, std::string_view msg, T... args) {
    auto str = std::format(msg, args...);
    print_with_severity(str, severity);
}

template <typename... T>
void report_error(Severity severity, ErrorCode err_code, T... args) {
    auto msg = get_error_msg(err_code);
    auto str = std::format(msg, args...);
    print_with_severity(str, severity);
}

template <typename... T>
void report_error_and_exit(Severity severity, std::string_view msg, T... args) {
    auto str = std::format(msg, args...);
    print_with_severity(str, severity);
    std::exit(-1);
}

template <typename... T>
void report_error_and_exit(Severity severity, ErrorCode err_code, T... args) {
    auto msg = get_error_msg(err_code);
    auto str = std::vformat(msg, std::make_format_args(args...));
    print_with_severity(str, severity);
    std::exit(-1);
}

} // namespace jb
