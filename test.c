int main() {
    int a = 0;

    for(int i = 0; i < 10; i += 1) {
        if(i % 2 == 0)
            a += i;
        else
            a -= 1;
    }

    return a;
}

// int main() {
//     struct s {
//         int a;
//         int b;
//     } s;
//     s.a = 10;
//     return s.a;
// }
