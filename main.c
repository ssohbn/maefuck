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

int main(int argc, char *argv[]) {
  if (argc != 2) {

    printf("Expected filename.\n");
    return 1;
  };

  Stack jumps = newStack(5);
  cell cells[4096] = {0};
  cell *currentCell = cells;

  // argv[0] is the program name
  char *filename = argv[1];
  FILE *bf_file = fopen(filename, "r");

  if (bf_file == NULL) {
    printf("Failed to open file");
    return -1;
  }

  // using globals for these instead of a function call is real nasty
  int skipping = 0;
  int skips = 0;

  char input;

  long offsets; // offset buffer

  while ((input = fgetc(bf_file))) {
    if (input == EOF)
      break;
    // printCells(cells, 10);
    // printf("instruction: %c: %ld\n", input, ftell(bf_file));
    // printf("currentCell: %p\n", (void*) currentCell);

    while (skipping) {
      switch (input) {

      case '[':
        skips++;
        break;

      case ']':
        // early exit stop skipping, we have found matching brace
        if (skips == 0) {
          // printf("found matching brace at %ld\n", ftell(bf_file));
          skipping = 0;
          break;
        }

        skips--;
        break;

      default:
        // printf("skipping over %c\n", input);
        break;
      }

      input = fgetc(bf_file);
    }

    switch (input) {
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

      offsets = peekStack(&jumps);
      // currentCell = (cell *)offsets.cellAddress;
      int offset = offsets - ftell(bf_file);
      // printf("current: %ld\n", ftell(bf_file));
      // printf("offset: %d\n", offset);
      // printf("going to: %ld\n", offsets.file_position);

      // goes here and input reads NEXT character.
      // need to go back so that we can readd positon to stack after pop
      fseek(bf_file, offset, SEEK_CUR);
      break;

    case '[':
      // Jump past the matching ] if the cell at the pointer is 0
      if (*currentCell == 0) {
        skipping = 1;
        break;
      }

      offsets = ftell(bf_file);

      pushStack(&jumps, offsets);
      break;

    default:
      // comment
      break;
    }
  }

  fclose(bf_file);
}
