#ifndef JAB_LINK_WINDOWS_PE_H
#define JAB_LINK_WINDOWS_PE_H

#include "jab.h"

#include <iostream>
#include <vector>
#include <string>
#include <format>

#include <windows.h>

namespace jab {

void link_coff_files(std::string, std::vector<std::string>,
                     bool print_linker_command = false);

};

#endif // JAB_LINK_WINDOWS_PE_H
