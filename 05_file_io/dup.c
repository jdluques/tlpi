#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

int dup(int oldfd) { return fcntl(oldfd, F_DUPFD, 0); }

int dup2(int oldfd, int newfd) {
  if (oldfd == newfd) {
    if (fcntl(oldfd, F_GETFL) == -1) {
      errno = EBADF;
      return -1;
    }
    return newfd;
  }

  close(newfd);

  int fd = fcntl(oldfd, F_DUPFD, newfd);
  if (fd == -1) {
    return -1;
  }

  if (fd != newfd) {
    close(fd);
    return -1;
  }

  return newfd;
}
