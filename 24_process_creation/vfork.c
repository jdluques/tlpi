#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  char *filepath = "./generated/file";

  int fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    perror("Error opening file");
    return EXIT_FAILURE;
  }

  int bytes_written = write(fd, "Hello, world!!!", 15);
  if (bytes_written == -1) {
    perror("Error writing to file");
    return EXIT_FAILURE;
  }

  if (lseek(fd, 0, SEEK_SET) == -1) {
    perror("Error setting file offset to 0");
    return EXIT_FAILURE;
  }

  switch (vfork()) {
  case -1:
    perror("Error forking with vfork");
    return EXIT_FAILURE;

  case 0:
    printf("Child process closing file descriptor %d\n", fd);
    if (close(fd) == -1) {
      perror("Error closing file");
      _exit(EXIT_FAILURE);
    }
    if (unlink(filepath) == -1) {
      perror("Error unlinking filename");
      _exit(EXIT_FAILURE);
    }
    printf("Child process successfully closed file descriptor %d\n", fd);

    _exit(EXIT_SUCCESS);

  default:
    sleep(3);

    printf("Parent process reading from file descriptor %d\n", fd);

    char buf[15];
    int bytes_read = read(fd, buf, 15);
    if (bytes_read < 0) {
      perror("Error reading file. File descriptor broken by child");
      return EXIT_FAILURE;
    }

    printf("Parent process successfully read from file descriptor %d: '%.*s'\n",
           fd, bytes_read, buf);
  }

  return EXIT_SUCCESS;
}
