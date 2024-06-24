// command-line driver

#include "Common.h"
#include "jb.h"
#include "error_reporter.h"

#include <charconv>
#include <optional>
#include <string_view>
#include <iostream>

using namespace jb;
using namespace std::literals;

std::optional<int> parse_int(const char* start, const char* end, int base = 10) {
	int result{};
	auto [ptr, ec] { std::from_chars(start, end, result, base) };

	if(ec == std::errc())
		return result;
	else
		return {};
}

CompileOptions parse_cmd_args(int argc, char* argv[]) {

	CompileOptions options;
	//options.set_default(get_host_os());
	std::vector<std::string_view> args(argv, argv + argc);

	for(auto str: args) {
		if(str.starts_with("-O"sv)) {
			if(str[2] == 's')
				options.opt = OptLevel::Os;
			else {
				auto res = parse_int(str.data() + 2, str.data() + str.size());
				auto val = res.value();
				if(!res || res < 0)
					report_error_and_exit(Severity::fatal, ErrorCode::f1000, str);
				// clamp opt level at max
				if(val >= 3)
					val = 2;
				options.opt = (OptLevel)val;
			}
		}
	}
	return options;
}

int main(int argc, char* argv[]) {
	auto args = parse_cmd_args(argc, argv);

	std::cout << (int)args.opt << "\n";
	std::cout << (int)args.debug << "\n";	

}
