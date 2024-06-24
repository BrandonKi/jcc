#pragma once

#include "jb.h"

#include <iostream>
#include <vector>
#include <string>
#include <format>

#include <windows.h>

namespace jb {

void link_coff_files(std::string, std::vector<std::string>,
                     bool print_linker_command = false);

};
