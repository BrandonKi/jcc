// #define CONCAT(a, b) a##b

// int main() {
//     int x = 1;
//     int y = 2;
//     int xy = 42;
//     return CONCAT(x +x, y);
// }

// extern int printf(char*);

// #define hash_hash # ## #
// #define mkstr(a) # a
// #define in_between(a) mkstr(a)
// #define join(c, d) in_between(c hash_hash d)
// int main() {
//     char *p = join(x, y);
//     return printf(p);
// }
// char p[] = "x ## y";



// #define SQUARE(x) ((x) * (x))

// int main() {
//     int num = 5;
//     return SQUARE(num);
// }

#define SCHAR_MIN   (-128)

int main() {
    return SCHAR_MIN;
}


// #include <stdlib.h>

// int test() {
//     int *i = malloc(4);
//     *i = 10;
//     return *i;
// }

