
int main() {
    struct Int {
        int i;
    };
    struct Int integer;
    struct Int *ptr = &integer;
    (*ptr).i = 42;
    return (*ptr).i;
}


// #define STRINGIFY(x) #x

// int main() {
//     return printf("%s", STRINGIFY(hello));
// }


// #define PASTE(x, y) x##y

// int main() {
//     int var12 = 0;
//     printf("%d", PASTE(var, 12)); // Output: 0
//     return 0;
// }


// #define SCHAR_MIN   (-128)

// int main() {
//     return SCHAR_MIN;
// }

// #include <stdlib.h>

// int test() {
//     int *i = malloc(4);
//     *i = 10;
//     return *i;
// }

