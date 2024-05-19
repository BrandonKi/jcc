# jcc
C Compiler.

:)

Can use either the custom backend or llvm.

The custom backend is located in [jab/](jab/) which was previously a separate project([just-another-backend](https://github.com/BrandonKi/just-another-backend)).

## Sample
WIP, can currently handle stuff like this.

```c
int main() {
    int x = 10;
    int y = x * 2;
    int z = y - x + 5;
    return -(+y * +z) / -(+x);
}
```

## Spec/Reference
https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf

https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html

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
