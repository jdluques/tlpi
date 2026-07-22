#define _FILE_OFFSET_BITS 64

#define _LARGEFILE64_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MIN_LARGE_SIZE ((off_t)(3ULL * 1024 * 1024 * 1024)) // 3 GiB

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Error: Insufficient arguments.\n");
    printf("Usage: %s <pathname> <offset>\n", argv[0]);
    return EXIT_FAILURE;
  }

  int fd = open(argv[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    perror("Error opening large file");
    return EXIT_FAILURE;
  }

  char *endptr;
  off_t off = strtoll(argv[2], &endptr, 10);
  if (endptr == argv[2]) {
    fprintf(stderr, "No digits found in argv[2]: %s", argv[2]);
    if (close(fd) == -1) {
      perror("Error closing large file");
    }
    return EXIT_FAILURE;
  }
  if (off < MIN_LARGE_SIZE) {
    fprintf(stderr, "Offset must be equal or larger than 3 GB");
    if (close(fd) == -1) {
      perror("Error closing large file");
    }
    return EXIT_FAILURE;
  }

  if (lseek(fd, off, SEEK_SET) == -1) {
    perror("Error setting offset of large file");
    if (close(fd) == -1) {
      perror("Error closing large file");
    }
    return EXIT_FAILURE;
  }

  if (write(fd, "test", 4) == -1) {
    perror("Error writing to large file");
    if (close(fd) == -1) {
      perror("Error closing large file");
    }
    return EXIT_FAILURE;
  }

  if (close(fd) == -1) {
    perror("Error closing large file");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
