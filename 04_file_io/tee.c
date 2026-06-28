#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifndef BUF_SIZE
#define BUF_SIZE 1024
#endif

int close_fds(int *fds, size_t fd_count)
{
  int status = 0;
  
  for (size_t i = 1; i < fd_count; i++) {
    if (close(fds[i]) == -1) {
      perror("Error closing file");
      status = -1;
    }
  }

  return status;
}

int open_fds(int *fds, char *argv[], int argc)
{
  int fd_count = 1;
  
  for (int i = 1; i < argc; i++) {
    int flags = O_WRONLY | O_CREAT;
    mode_t mode = S_IRUSR | S_IWUSR |
                  S_IRGRP | S_IWGRP |
                  S_IROTH | S_IWOTH;

    if (strcmp(argv[i], "-a") == 0) {
      flags |= O_APPEND;
      
      while (strcmp(argv[i], "-a") == 0) {
        i++;
        if (i >= argc) {
          fprintf(stderr, "Error: '-a' flag provided without a subsequent filepath.\n");
          return -1;
        }
      }
    } else {
      flags |= O_TRUNC;
    }

    if ((fds[fd_count] = open(argv[i], flags, mode)) == -1) {
      perror("Error opening file");
      close_fds(fds, fd_count);
      return -1;
    }

    fd_count++;
  }

  return fd_count;
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(stderr, "Error: insufficient arguments.\n");
    printf("Usage: %s <filepath>...\n", argv[0]);
    return EXIT_FAILURE;
  }

  int *fds = malloc(sizeof(int) * argc);
  if (fds == nullptr) {
    perror("Error allocating memory for file descriptors array");
    return EXIT_FAILURE;
  }

  fds[0] = STDOUT_FILENO;

  int fd_count = open_fds(fds, argv, argc);
  if (fd_count == -1) {
    free(fds);
    return EXIT_FAILURE;
  }

  char buf[BUF_SIZE];
  ssize_t num_read, num_written;
  
  while ((num_read = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
    for (int i = 0; i < fd_count; i++) {
      for (ssize_t total_written = 0; total_written < num_read;) {
        num_written = write(
          fds[i],
          buf + total_written,
          num_read - total_written
        );

        if (num_written == -1) {
          if (errno == EINTR) {
            continue;
          }

          perror("Error writing to file");

          close_fds(fds, fd_count);
          free(fds);
          
          return EXIT_FAILURE;
        }

        total_written += num_written;
      }
    }
  }
  if (num_read == -1) {
    perror("Error reading from stdin");
    
    close_fds(fds, fd_count);
    free(fds);
    
    return EXIT_FAILURE;
  }

  if (close_fds(fds, fd_count) == -1) {
    free(fds);
    return EXIT_FAILURE;
  }

  free(fds);

  return EXIT_SUCCESS;
}
