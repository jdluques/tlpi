#include <stdlib.h>
#include <unistd.h>

int main() {
  // Beginning of the source code
  int pid = fork();
  if (pid == 0) {
    abort(); // Generate core dump in child process
  }
  // Rest of the source code
}
