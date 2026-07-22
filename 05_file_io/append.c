#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Insufficient arguments.\n");
    printf("Usage: %s <pathname>\n", argv[0]);
    return EXIT_FAILURE;
  }

  int fd = open(argv[1], O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    perror("Error opening file");
    return EXIT_FAILURE;
  }

  if (lseek(fd, 0, SEEK_SET) == -1) {
    perror("Error moving offset of file to the beginning of the file");
    if (close(fd) == -1) {
      perror("Error closing file");
    }
    return EXIT_FAILURE;
  }

  char buffer[] = "Hello, world!!!\n";

  if (write(fd, &buffer, strlen(buffer)) == -1) {
    perror("Error writing to file");
    if (close(fd) == -1) {
      perror("Error closing file");
    }
    return EXIT_FAILURE;
  }

  if (close(fd) == -1) {
    perror("Error closing file");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
