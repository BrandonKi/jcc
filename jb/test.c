
int f(int a, int b) {
    int c;
    if (a < b) {
        c = a;
    } else {
        c = b;
    }

    if(a == 0) {
        c = 999;
    } 
    return c;
}


// int c1();
// int c2();

// void fn(int);

// int test() {
//     int x = c1();
//     fn(x);
//     x = c2();
//     fn(x);
//     return x;
// }


// int main() {
//     int x = 17;
//     int y = 6;
//     int z = 0;
//     if(y < x)
//         z = x + 3;
//     return z + x;
// }