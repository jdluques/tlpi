#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_PATH_SIZE 100

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Insufficient arguments.\n");
    printf("Usage: %s <filename> [newfd]\n", argv[0]);
    return EXIT_FAILURE;
  }

  char pathname[MAX_PATH_SIZE] = "generated/";
  strcat(pathname, argv[1]);

  int oldfd = open(pathname, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (oldfd == -1) {
    fprintf(stderr, "Error opening file in %s: %s\n", argv[1], strerror(errno));
  }

  int newfd;
  if (argc == 2) {
    newfd = dup(oldfd);
  } else {
    newfd = dup2(oldfd, atoi(argv[2]));
  }
  if (newfd == -1) {
    fprintf(stderr, "Error duplicating old file descriptor (%d): %s\n", oldfd,
            strerror(errno));
    return EXIT_FAILURE;
  }

  int oldfd_off = lseek(oldfd, 0, SEEK_SET);
  if (oldfd_off == -1) {
    fprintf(stderr, "Error obtaining offset of old file descriptor (%d): %s\n",
            oldfd, strerror(errno));
    return EXIT_FAILURE;
  }

  int newfd_off = lseek(newfd, 0, SEEK_END);
  if (newfd_off == -1) {
    fprintf(stderr, "Error obtaining offset of new file descriptor (%d): %s\n",
            newfd, strerror(errno));
    return EXIT_FAILURE;
  }

  if (oldfd_off == newfd_off) {
    printf("The offset of the duplicated file descriptors are the same.\n");
  } else {
    printf("The offset of the duplicated file descriptors are not the same.\n");
  }

  int oldfd_flags = fcntl(oldfd, F_GETFL);
  if (oldfd_flags == -1) {
    fprintf(stderr,
            "Error obtaining status flags of old file descriptor (%d): %s\n",
            oldfd, strerror(errno));
    return EXIT_FAILURE;
  }

  int newfd_flags = fcntl(newfd, F_GETFL);
  if (newfd_flags == -1) {
    fprintf(stderr,
            "Error obtaining status flags of new file descriptor (%d): %s\n",
            newfd, strerror(errno));
    return EXIT_FAILURE;
  }

  if (oldfd_flags == newfd_flags) {
    printf(
        "The status flags of the duplicated file descriptors are the same.\n");
  } else {
    printf("The status flags of the duplicated file descriptors are not the "
           "same.\n");
  }

  if (close(oldfd) == -1) {
    fprintf(stderr, "Error closing old file descriptor (%d): %s\n", oldfd,
            strerror(errno));

    if (close(newfd) == -1) {
      fprintf(stderr, "Error closing new file descriptor (%d): %s\n", newfd,
              strerror(errno));
    }

    return EXIT_FAILURE;
  }
  if (close(newfd) == -1) {
    fprintf(stderr, "Error closing new file descriptor (%d): %s\n", newfd,
            strerror(errno));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
