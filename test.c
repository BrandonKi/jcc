extern int printf(char *);
extern void puts(char *);

int main() {
    char *a = "Hello World!";
    int result = printf(a);
    puts("");
    return result;
}

// int main() {
//     struct s {
//         int a;
//         int b;
//     } s;
//     s.a = 10;
//     return s.a;
// }
