#include <stdio.h>
#include <unistd.h>

int main() {
  fork();
  fork();
  fork();

  printf("Process with id %d\n", getpid());

  return 0;
}
