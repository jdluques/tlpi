#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef BUF_SIZE
#define BUF_SIZE 1024
#endif

int main(int argc, char *argv[])
{
  if (argc != 3) {
    fprintf(stderr, "Error: Insufficient arguments.\n");
    printf("Usage: %s <input_path> <output_path>\n", argv[0]);
    return EXIT_FAILURE;
  }

  int input_fd = open(argv[1], O_RDONLY);
  if (input_fd == -1) {
    perror("Error opening input file");
    return EXIT_FAILURE;
  }

  int    open_flags = O_CREAT | O_WRONLY | O_TRUNC;
  mode_t file_perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                      S_IROTH | S_IWOTH;

  int output_fd = open(argv[2], open_flags, file_perms);
  if (output_fd == -1) {
    perror("Error opening output file");
    
    close(input_fd);

    return EXIT_FAILURE;
  }

  char buf[BUF_SIZE];
  while (1) {
    ssize_t num_read = read(input_fd, buf, BUF_SIZE);
    int saved_errno = errno;
    
    if (num_read == -1) {
      if (saved_errno == EINTR) {
        continue;
      }

      perror("Fatal error reading from input");
      
      close(input_fd);
      close(output_fd);

      return EXIT_FAILURE;
    }
    if (num_read == 0) {
      break;
    }

    for (ssize_t total_written = 0; total_written < num_read;) {
      ssize_t num_written = write(
        output_fd,
        buf + total_written,
        num_read - total_written
      );
      
      if (num_written == -1){
        saved_errno = errno;
        if (saved_errno == EINTR) {
          continue;
        }

        perror("Error writing entire buffer");
      
        close(input_fd);
        close(output_fd);

        return EXIT_FAILURE;
      }

      total_written += num_written;
    }
  }

  if (close(input_fd) == -1) {
    perror("Error closing input file");
    return EXIT_FAILURE;
  }
  if (close(output_fd) == -1) {
    perror("Error closing output file");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
