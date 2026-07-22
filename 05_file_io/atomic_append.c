#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Insufficient arguments.\n");
    printf("Usage:\n"
           "%s <filename><num_bytes>\n"
           "%s <filename> <num_bytes> [x]\n",
           argv[0], argv[0]);
    return EXIT_FAILURE;
  }

  int num_bytes = atoi(argv[2]);

  int flags = O_RDWR | O_CREAT;
  if (argc == 3) {
    flags |= O_APPEND;
  }

  int fd = open(argv[1], flags, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    perror("Error opening file");
    return EXIT_FAILURE;
  }

  char buf[64];
  for (int i = 0; i < num_bytes; i++) {
    if (argc == 4) {
      if (lseek(fd, 0, SEEK_END) == -1) {
        perror("Error setting offset to EOF");
        return EXIT_FAILURE;
      }
    }

    if (write(fd, buf, 1) == -1) {
      perror("Error writing byte to file");
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
