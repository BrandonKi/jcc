int identity(int x) {
    int *y = &x;
    int z = *y;
    return *&z;
}

int main() {
    return identity(99);
}

// int main() {
//     struct s {
//         int a;
//         int b;
//     } s;
//     s.a = 10;
//     return s.a;
// }
