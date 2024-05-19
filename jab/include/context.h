#ifndef JAB_CONTEXT_H
#define JAB_CONTEXT_H

#include "jab.h"

#include "module_builder.h"
#include "jit_env.h"
#include "arch/x86_64/mdir_gen.h"

namespace jab {

class Context {
public:
	CompileOptions options;
	std::vector<std::string> object_files;
	
	Context(): options{}, object_files{} {}
	Context(CompileOptions options): options{options}, object_files{} {}

	ModuleBuilder* new_module_builder(std::string);

	BinaryFile* compile(ModuleBuilder*);
	
	void write_object_file(BinaryFile*);
	void link_objects();
	
	JITEnv* new_jit_env(ModuleBuilder*, CompileOptions = {});

};

} // namespace jab

#endif // JAB_CONTEXT_H
