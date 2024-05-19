#ifndef JAB_JIT_ENV_H
#define JAB_JIT_ENV_H

#include "jab.h"

namespace jab {

class JITEnv {
public:
	JITEnv(std::vector<byte> bin): bin{bin} {}

	i32 run_main();

	// TODO use a symtab to find the function location first
	template<typename T, typename... U>
	auto run_function(std::string name, U... args) {
	    void* block = alloc_memory(bin.size());
		memcpy(block, bin.data(), bin.size());
		using exe = T;
		exe func = (exe)make_executable(block);
		auto ret = func(args...);
		dealloc(block, bin.size());
		return ret;
	}
	
private:
	std::vector<byte> bin;

	void* alloc_memory(size_t size);
	void dealloc(void *block, size_t size);
	void* make_executable(void *buf);

};

} // namespace jab

#endif // JAB_JIT_ENV_H
