int main() {
    struct IntPair {
        long long a;
        int b;
    };
    struct IntPair s;
    s.a = 5;
    s.b = 10;
    return s.a + s.b;
}

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

