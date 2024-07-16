#define COND1

int main() {
#ifndef COND1
#ifdef COND2
    return 1;
#else
    return 2;
#endif
#else
    return 100;
#endif
    return -1;
}

// #define ii z
// #define z ii
// #undef z

// int main() {
//     return z;
// }


// int main() {
//     struct s {
//         int a;
//         int b;
//     } s;
//     s.a = 10;
//     return s.a;
// }
