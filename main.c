#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef char cell;

typedef struct Stack {
  long *file_positions;
  uintptr_t offset;
  int capacity;
} Stack;

Stack newStack(int stackSize) {
  Stack stack;
  stack.file_positions = (long *)malloc(stackSize * sizeof(long));
  stack.offset = 0;
  stack.capacity = stackSize;

  return stack;
};

void pushStack(Stack *stack, long offsets) {
  // if we are going over the max amt just double the capacity
  if (stack->offset >= stack->capacity * sizeof(stack->offset)) {
    printf("reallocating\n");
    stack->capacity = stack->capacity * 2;

    stack->file_positions =
        (long *)realloc(stack->file_positions, stack->capacity * sizeof(long));
  }

  // insert item
  *(stack->file_positions + stack->offset) = offsets;

  // go forward
  stack->offset++;
}

long popStack(Stack *stack) {
  // go back
  stack->offset--;

  // copy out value
  long output = *(stack->file_positions + stack->offset);

  // set new value
  long offsets;

  *(stack->file_positions + stack->offset) = offsets;

  return output;
}

long peekStack(Stack *stack) {
  // go back
  stack->offset--;

  // copy out value
  long output = *(stack->file_positions + stack->offset);

  stack->offset++;
  return output;
}

void printStack(Stack *stack) {
  for (int i = 0; i < stack->offset; i++) {
    printf("%ld\n", *stack->file_positions);
  }
}

void printCells(cell *cells, int readct) {
  for (int i; i < readct; i++) {
    printf("%u-", cells[i]);
  }
  printf("\n");
};

// returns 1 if found
// returns 0 if reached EOF
int skipForward(FILE *bf_file) {

  int skips = 0;

  char input;
  while ((input = getc(bf_file))) {
    switch (input) {

    case '[':
      skips++;
      break;

    case ']':
      // early exit stop skipping, we have found matching brace
      if (skips == 0) {
        return 1;
      }

      skips--;
      break;

    case EOF:
      return 0;

    default:
      // printf("skipping over %c\n", input);
      break;
    }
  }

  return -1;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {

    printf("Expected filename.\n");
    return 1;
  };

  Stack jumps = newStack(10);
  cell cells[4096] = {0};
  cell *currentCell = cells;

  // argv[0] is the program name
  char *filename = argv[1];
  FILE *bf_file = fopen(filename, "r");

  if (bf_file == NULL) {
    printf("Failed to open file");
    return -1;
  }

  char input;

  long loop_entry; // offset buffer

  while ((input = fgetc(bf_file))) {
    switch (input) {
    case EOF:
      // printf("reached end of file\n");
      return 0;

    case '?':
      printCells(cells, 10);
      break;

    case '>':
      // Move the pointer to the right
      currentCell++;
      break;

    case '<':
      // Move the pointer to the left
      currentCell--;
      break;

    case '+':
      // Increment the memory cell at the pointer
      (*currentCell)++;
      break;

    case '-':
      // Decrement the memory cell at the pointer
      (*currentCell)--;
      break;

    case '.':
      // Output the character signified by the cell at the pointer
      printf("%c", *currentCell);
      break;

    case ',':
      // Input a character and store it in the cell at the pointer
      printf("\ninput? ");
      char userInput = getchar();
      char c;
      while ((c = getchar() != '\n' && c != EOF))
        ;

      *currentCell = userInput;
      break;

    case ']':
      // Jump back to the matching [ if the cell at the pointer is nonzero
      if (*currentCell == 0) {
        popStack(&jumps);
        break;
      }

      loop_entry = peekStack(&jumps);
      // currentCell = (cell *)offsets.cellAddress;
      int to_loop_entry = loop_entry - ftell(bf_file);

      fseek(bf_file, to_loop_entry, SEEK_CUR);
      break;

    case '[':
      // Jump past the matching ] if the cell at the pointer is 0
      if (*currentCell == 0) {
        int ok = skipForward(bf_file);
        if (!ok) {
          printf("catastrophic error\n");
          return 1;
        }

        break;
      }

      loop_entry = ftell(bf_file);

      pushStack(&jumps, loop_entry);
      break;

    default:
      // comment
      break;
    }
  }

  fclose(bf_file);
}
