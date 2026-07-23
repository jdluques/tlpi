#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_IOV 1024

struct iovec {
  void *iov_base;
  size_t iov_len;
};

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
  if (iovcnt < 0 || MAX_IOV < iovcnt) {
    return -1;
  }

  ssize_t total_bytes_read = 0;

  for (int i = 0; i < iovcnt; i++) {
    void *buffer = iov[i].iov_base;
    size_t len = iov[i].iov_len;

    if (len == 0) {
      continue;
    }

    ssize_t bytes_read = read(fd, buffer, len);
    if (bytes_read == -1) {
      if (errno == EINTR) {
        i--;
        continue;
      }
      return (total_bytes_read > 0) ? total_bytes_read : -1;
    }

    total_bytes_read += bytes_read;

    if (bytes_read < len) {
      break;
    }
  }

  return total_bytes_read;
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
  if (iovcnt < 0 || MAX_IOV < iovcnt) {
    return -1;
  }

  ssize_t total_bytes_written = 0;

  for (int i = 0; i < iovcnt; i++) {
    void *buffer = iov[i].iov_base;
    size_t len = iov[i].iov_len;

    while (len > 0) {
      ssize_t bytes_written = write(fd, buffer, len);
      if (bytes_written == -1) {
        if (errno == EINTR) {
          continue;
        }
        return (total_bytes_written > 0) ? total_bytes_written : -1;
      }

      total_bytes_written += bytes_written;
      buffer += bytes_written;
      len -= bytes_written;
    }
  }

  return total_bytes_written;
}
