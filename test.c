#define ii z
#define z ii
#undef z

int main() {
    return z;
}

// int main() {
//     return 1;
// }

// int main() {
//     struct s {
//         int a;
//         int b;
//     } s;
//     s.a = 10;
//     return s.a;
// }
