# Just a C Compiler (jcc)

C Compiler.

Can use either the custom backend or llvm.

The custom backend is located in [jb/](jb/) which was previously a separate project([jb](https://github.com/BrandonKi/just-another-backend)).

## Examples

WIP, can currently handle a decent amount of stuff (functions, pointers, if/else, for, do/while, etc.). It even supports a decent amount of preprocessor features so far.

Take a look in the [tests/](tests/) directory or commit history for a fuller picture of how much is supported.

The following is a very small example that correctly runs/compiles.
```c
#include <stdbool.h>

extern int printf(char*);

bool isPerfect(int num) {
    int sum = 0;
    for (int i = 1; i < num; i++) {
        if (num % i == 0) {
            sum += i;
        }
    }
    return sum == num;
}

int main() {
    printf("Hello From JCC!");
    return isPerfect(28); // returns 1
}
```

## Spec/Reference

[C Spec](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)

[C preprocessor](https://www.spinellis.gr/blog/20060626/cpp.algo.pdf)

[Intel x86_64 Instruction set](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)

## Build

Replace `CMAKE_PREFIX_PATH` with your `path/to/llvm`

## Build(Visual Studio)

```cmake
cmake "-DCMAKE_PREFIX_PATH:STRING=C:/Program Files/llvm-16.0.6-windows-amd64-msvc16-msvcrt-dbg" -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -B/build -G "Visual Studio 17 2022"
```

## Build(Clang+Ninja)

```cmake
cmake "-DCMAKE_PREFIX_PATH:STRING=C:/Program Files/llvm-16.0.6-windows-amd64-msvc16-msvcrt-dbg" -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -B/build -GNinja
```
