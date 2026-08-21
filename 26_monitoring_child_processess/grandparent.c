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
		switch (fork()) {
		case -1:
			perror("fork");
			_exit(EXIT_FAILURE);
		case 0:
			sleep(3);
			printf("The PID of the parent process of the grandchild proces after the parent dies "
				   "and when the grandparent hasn't terminated yet is %d\n",
				   getppid());

			sleep(6);
			printf("The PID of the parent process of the grandchild proces when the grandparent "
				   "has terminated is %d\n",
				   getppid());

			_exit(EXIT_SUCCESS);
		default:
			_exit(EXIT_SUCCESS);
		}

	default:
		printf("The PID of the grandparent process is %ld\n", (long)getpid());
		sleep(5);
		return EXIT_SUCCESS;
	}
}
