# jcc

C Compiler.

:)

Can use either the custom backend or llvm.

The custom backend is located in [jab/](jab/) which was previously a separate project([just-another-backend](https://github.com/BrandonKi/just-another-backend)).

## Sample

WIP, can currently handle a decent amount of stuff (functions, pointers, if/else, for, do/while, etc.).
Take a look in the tests directory or commit history for a fuller picture of how much is supported.

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
