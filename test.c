#include <stdbool.h>
extern int printf(char*);

// int print_board(int board[][8]) {
int print_board(int **board) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j])
                printf("Q ");
            else
                printf(". ");
        }
        printf("\n");
    }
    printf("\n\n");
}

// int conflict(int board[][8], int row, int col) {
bool conflict(int **board, int row, int col) {
    for (int i = 0; i < row; i++) {
    if (board[i][col])
        return true;
    int j = row - i;
    if (0 < col - j + 1)
        if (board[i][col - j])
        return true;
    if (col + j < 8)
        if (board[i][col + j])
        return true;
    }
    return false;
}

// int solve(int board[][8], int row) {
void solve(int **board, int row) {
    if (row == 8) {
        print_board(board);
        return;
    }
    for (int i = 0; i < 8; i++) {
        if (!conflict(board, row, i)) {
            board[row][i] = 1;
            solve(board, row + 1);
            board[row][i] = 0;
        }
    }
}

int main() {
    int board[64];
    for (int i = 0; i < 64; ++i) {
        board[i] = 0;
    }
    solve(board, 0);
}


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

