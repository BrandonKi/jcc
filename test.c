#include <stdbool.h>
extern int printf(char*);

void print_board(int *board) {
//   printf("p start\n");
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (board[i * 8 + j] == 1)
        printf("Q ");
      else
        printf(". ");
    }
    printf("\n");
  }
  printf("\n\n");
//   printf("p end\n");
  return;
}

bool conflict(int *board, int row, int col) {
//   printf("c start\n");
  for (int i = 0; i < row; i++) {
    if (board[i * 8 + col] == 1)
      return true;

    int j = row - i;
    if ((col - j) >= 0)
        if(board[i * 8 + (col - j)] == 1)
            return true;

    if ((col + j) < 8)
        if(board[i * 8 + (col + j)] == 1)
            return true;
  }
  return false;
}

int solve(int *board, int row) {
//   printf("s start\n");
  if (row == 8) {
    print_board(board);
    // printf("s end\n");
    return 1;
  }
  int solutions = 0;
  for (int i = 0; i < 8; i++) {
    if (conflict(board, row, i) == false) {
      board[row * 8 + i] = 1;
      solutions += solve(board, row + 1);
      board[row * 8 + i] = 0;
    }
  }
//   printf("s end\n");
  return solutions;
}

int main() {
  int board[64];
  int *board_ptr = board;
  for (int i = 0; i < 64; i++) {
    board[i] = 0;
  }
  solve(board_ptr, 0);
  printf("DONE!");
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

