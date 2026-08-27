#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ;

char *search_in_path(const char *file)
{
	if (strchr(file, '/') != NULL) {
		return strdup(file);
	}

	char *path_env = getenv("PATH");
	if (!path_env) {
		return NULL;
	}

	char *path_copy = strdup(path_env);
	char *dir = strtok(path_copy, ":");

	while (dir != NULL) {
		char *full_path = malloc(strlen(dir) + strlen(file) + 2);
		sprintf(full_path, "%s/%s", dir, file);

		if (access(full_path, X_OK) == 0) {
			free(path_copy);
			return full_path;
		}

		free(full_path);
		dir = strtok(NULL, ":");
	}

	free(path_copy);
	return NULL;
}

int count_list(va_list args, const char *arg0)
{
	int args_count = 0;

	const char *temp = arg0;
	while (temp != NULL) {
		args_count++;
		temp = va_arg(args, const char *);
	}

	return args_count;
}

char **vector_from_list(const int args_count, va_list args, const char *arg0)
{
	char **argv = malloc(sizeof(char *) * (args_count + 1));
	if (argv == NULL) {
		return NULL;
	}

	argv[0] = (char *)arg0;
	for (int i = 1; i < args_count; i++) {
		argv[i] = va_arg(args, char *);
	}
	argv[args_count] = NULL;

	return argv;
}

int execlp(const char *file, const char *arg0, ...)
{
	if (file == NULL || arg0 == NULL) {
		errno = EINVAL;
		return -1;
	}

	va_list args;

	va_start(args, arg0);
	int args_count = count_list(args, arg0);
	va_end(args);

	va_start(args, arg0);
	char **argv = vector_from_list(args_count, args, arg0);
	va_end(args);
	if (argv == NULL) {
		return -1;
	}

	char *complete_path = search_in_path(file);
	if (complete_path == NULL) {
		free(argv);
		return -1;
	}

	execve(complete_path, argv, environ);

	free(complete_path);
	free(argv);
	return -1;
}
