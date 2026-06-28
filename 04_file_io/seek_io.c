#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
  if (argc < 3) {
    fprintf(stderr, "Error: insufficient arguments.\n");
    printf("Usage: %s <input_path> <io_op ...>\n", argv[0]);
    return EXIT_FAILURE;
  }

  int fd = open(
    argv[1],
    O_RDWR | O_CREAT,
    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
  );
  if (fd == -1) {
    perror("Error opening file");
    return EXIT_FAILURE;
  }

  for (int ap = 2; ap < argc; ap++) {
    int len;
    char *endptr = nullptr;
    switch (argv[ap][0]) {
      case 'r':
      case 'R':
        errno = 0;

        len = strtol(&argv[ap][1], &endptr, 10);
        if (*endptr != '\0') {
          fprintf(stderr, "Error during conversion from str to number: found non digit %c in %s", *endptr, &argv[ap][1]);
          close(fd);
          return EXIT_FAILURE;
        }
        if (errno != 0) {
          perror("Error during conversion from str to number");
          close(fd);
          return EXIT_FAILURE;
        }

        char *buf = malloc(len);
        if (buf == nullptr) {
          perror("Error allocating memory for buffer");
          close(fd);
          return EXIT_FAILURE;
        }

        ssize_t num_read = read(fd, buf, len);
        if (num_read == -1) {
          perror("Error reading from file");
          close(fd);
          return EXIT_FAILURE;
        }

        if (num_read == 0) {
          printf("%s: end-of-file\n", argv[ap]);
        } else {
          printf("%s: ", argv[ap]);
          for (int j = 0; j < num_read; j++) {
            if (argv[ap][0] == 'r') {
              printf("%c",
                     isprint((unsigned char) buf[j]) ? buf[j] : '?');
            } else {
              printf("%02x ", (unsigned int) buf[j]);
            }
          }
          printf("\n");
        }

        free(buf);

        break;
      
      case 'w':
        len = strlen(&argv[ap][1]);
        
        ssize_t total_written;
        for (total_written = 0; total_written < len;) {
          ssize_t num_written = write(
            fd,
            &argv[ap][1] + total_written,                     
            len - total_written
          );

          if (num_written == -1) {
            if (errno == EINTR) {
              continue;
            }

            perror("Error writing to file");
            close(fd);
            return EXIT_FAILURE;
          }

          total_written += num_written;
        }

        printf("%s: wrote %zd bytes\n", argv[ap], total_written);

        break;
      
      case 's':
        errno = 0;

        off_t offset = strtol(&argv[ap][1], &endptr, 10);
        if (*endptr != '\0') {
          fprintf(stderr, "Error during conversion from str to number: found non digit %c in %s", *endptr, &argv[ap][1]);
          close(fd);
          return EXIT_FAILURE;
        }
        if (errno != 0) {
          perror("Error during conversion from str to number");
          close(fd);
          return EXIT_FAILURE;
        }

        if (lseek(fd, offset, SEEK_SET) == -1) {
          perror("Error adjusting file offset");
          close(fd);
          return EXIT_FAILURE;
        }

        printf("%s: seek succeded\n", argv[ap]);

        break;
  
      default:
        fprintf(stderr, "Argument %d must start with [rRws]: %s\n", ap, argv[ap]);
        return EXIT_FAILURE; 
    }
  }

  return EXIT_SUCCESS;
}
