#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char **environ;

int is_invalid_name(const char *name) {
  return name == NULL || *name == '\0' || strchr(name, '=') != NULL;
}

int setenv(const char *name, const char *value, int overwrite) {
  if (is_invalid_name(name) || value == NULL) {
    errno = EINVAL;
    return -1;
  }

  if (overwrite == 0 && getenv(name) != NULL) {
    return 0;
  }

  size_t name_len = strlen(name);
  size_t value_len = strlen(value);

  if (name_len > SIZE_MAX - 2 || value_len > SIZE_MAX - name_len - 2) {
    errno = ENOMEM;
    return -1;
  }

  size_t new_env_len = name_len + value_len + 2;

  char *new_env = malloc(new_env_len);
  if (new_env == NULL) {
    errno = ENOMEM;
    return -1;
  }
  snprintf(new_env, new_env_len, "%s=%s", name, value);

  if (putenv(new_env) != 0) {
    free(new_env);
    return -1;
  }

  return 0;
}

int unsetenv(const char *name) {
  if (is_invalid_name(name)) {
    errno = EINVAL;
    return -1;
  }

  if (environ == NULL) {
    return 0;
  }

  size_t name_len = strlen(name);
  char **read_ptr = environ;
  char **write_ptr = environ;

  while (*read_ptr != NULL) {
    if (strncmp(*read_ptr, name, name_len) == 0 &&
        (*read_ptr)[name_len] == '=') {
      read_ptr++;
    } else {
      *write_ptr = *read_ptr;
      write_ptr++;
      read_ptr++;
    }
  }

  *write_ptr = NULL;

  return 0;
}
