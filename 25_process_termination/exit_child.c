#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
	switch (fork()) {
	case -1:
		perror("fork");
		return EXIT_FAILURE;

	case 0:
		exit(-1);

	default: {
		int status;
		wait(&status);

		if (WIFEXITED(status)) {
			int child_exit_code = WEXITSTATUS(status);
			printf("Child's exit code was %d\n", child_exit_code);
			return EXIT_SUCCESS;
		} else {
			printf("Child did not exit normally");
			return EXIT_FAILURE;
		}
	}
	};
}
