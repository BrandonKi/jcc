int fn(int x, int y) {
    int z = y - x + 5;
    return -(y * z) / -(+x);
}

int main() {
    return fn(10, 20);
}

// int main() {
//     struct s {
//         int a;
//         int b;
//     } s;
//     s.a = 10;
//     return s.a;
// }
