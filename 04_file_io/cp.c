#define _GNU_SOURCE

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifndef BUF_SIZE
#define BUF_SIZE 1024
#endif

int duplicate_symlink(const char *src_symlink_path, const char *dst_symlink_path)
{
  char target[PATH_MAX];

  ssize_t len = readlink(src_symlink_path, target, PATH_MAX - 1);
  if (len == -1) {
    perror("Error reading source symlink");
    return -1;
  }

  target[len] = '\0';

  if (symlink(target, dst_symlink_path) == -1) {
    perror("Error creating destination symlink");
    return -1;
  }

  return 0;
}

int close_fds(int src_fd, int dst_fd)
{
  errno = 0;
  
  if (close(src_fd) == -1) {
    perror("Error closing source file");
  }
  if (close(dst_fd) == -1) {
    perror("Error closing destination file");
  }

  return errno == 0 ? 0 : -1;
}

int main(int argc, char *argv[])
{
  if (argc < 3) {
    fprintf(stderr, "Error: insufficient arguments.\n");
    printf("Usage: %s <source> <destination>\n", argv[0]);
    return EXIT_FAILURE;
  }

  char *src_path = argv[1],
       *dst_path = argv[2];

  struct stat src_file_info;
  if (lstat(src_path, &src_file_info) == -1) {
    perror("Error retrieving information of file");
    return EXIT_FAILURE;
  }

  // If file is a symlink it is handled in a different way
  if (S_ISLNK(src_file_info.st_mode)) {
    if (duplicate_symlink(src_path, dst_path) == -1) {
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  }

  int src_fd = open(src_path, O_RDONLY);
  if (src_fd == -1) {
    perror("Error opening source file");
    return EXIT_FAILURE;
  }

  int dst_fd = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, src_file_info.st_mode & 07777);
  if (dst_fd == -1) {
    perror("Error opening destination file");
    if (close(src_fd) == -1) {
      perror("Error closing source file");
    }
    return EXIT_FAILURE;
  }

  off_t src_filesize = src_file_info.st_size,
        offset       = 0;
  char  buf[BUF_SIZE];
  
  while (offset < src_filesize) {
    // Find where the next chunk of actual data starts
    off_t data_start = lseek(src_fd, offset, SEEK_DATA);
    if (data_start == -1) {
      if (errno == ENXIO) {
        break;
      }

      perror("Error seeking data source in file");
      close_fds(src_fd, dst_fd);
      return EXIT_FAILURE;
    }

    // Find where the next hole to the right of the data block starts
    off_t hole_start = lseek(src_fd, data_start, SEEK_HOLE);
    if (hole_start == -1) {
      perror("Error seeking hole in source file");
      close_fds(src_fd, dst_fd);
      return EXIT_FAILURE;
    }

    // Sync the offset pointers of both source and destination to match the start of the data segment
    // This will create a hole if necessary in destination
    if (lseek(src_fd, data_start, SEEK_SET) == -1) {
      perror("Error setting source file offset");
      close_fds(src_fd, dst_fd);
      return EXIT_FAILURE;
    }

    if (lseek(dst_fd, data_start, SEEK_SET) == -1) {
      perror("Error setting destination file offset");
      close_fds(src_fd, dst_fd);
      return EXIT_FAILURE;
    }

    // Copy the continous chunk of data from data_start to hole_start
    // Only works if hole_start > data_start, otherwise the current block is a hole
    off_t bytes_to_copy = hole_start - data_start;
    while (bytes_to_copy > 0) {
      size_t chunk_size = (bytes_to_copy > BUF_SIZE) ? BUF_SIZE : bytes_to_copy;
      ssize_t num_read = read(src_fd, buf, chunk_size);
      if (num_read == -1) {
        if (errno == EINTR) {
          continue;
        }

        perror("Error reading from source file");
        close_fds(src_fd, dst_fd);
        return EXIT_FAILURE;
      }

      for (ssize_t total_written = 0; total_written < num_read;) {
        ssize_t num_written = write(
          dst_fd,
          buf + total_written,
          num_read - total_written
        );
        
        if (num_written == -1) {
          if (errno == EINTR) {
            continue;
          }

          perror("Error writing to destination file");
          close_fds(src_fd, dst_fd);
          return EXIT_FAILURE;
        }

        total_written += num_written;
      }

      bytes_to_copy -= num_read;
    }

    // Move the offset to the end of the data block
    offset = hole_start;
  }
  
  // Force the destination file size to match the source file size (in case file ends in a hole)
  if (ftruncate(dst_fd, src_filesize) == -1) {
    perror("Error truncating destination file to source file size");
    close_fds(src_fd, dst_fd);
    return EXIT_FAILURE;
  }

  if (close_fds(src_fd, dst_fd) == -1) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
