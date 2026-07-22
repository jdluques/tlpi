#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PATH_SIZE 100

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Insufficient arguments.\n");
    printf("Usage: %s <filename>", argv[0]);
    return EXIT_FAILURE;
  }

  char pathname[MAX_PATH_SIZE] = "generated/";
  strcat(pathname, argv[1]);

  int fd1 = open(pathname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  int fd2 = dup(fd1);
  int fd3 = open(pathname, O_RDWR);

  // Empty

  write(fd1, "Hello,", 6);

  // Hello,

  write(fd2, "world", 6);

  // Hello,world

  lseek(fd2, 0, SEEK_SET);

  write(fd1, "HELLO,", 6);

  // HELLO,world

  write(fd3, "Gidday", 6);

  // Giddayworld
}
