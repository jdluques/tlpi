#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
	switch (fork()) {
	case -1:
		perror("fork");
		return EXIT_FAILURE;
	case 0:
		sleep(5);
		printf("The PID of the parent is %d\n", getppid());
		exit(EXIT_SUCCESS);
	default:
		printf("Parent process terminated\n");
		_exit(EXIT_SUCCESS);
	}
}
