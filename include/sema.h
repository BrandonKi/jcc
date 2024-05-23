#pragma once

#include "ast.h"

class Sema {
public:
    Sema();

    bool run_on(jcc::FileNode *);
};
