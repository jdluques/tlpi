#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define CMD_SIZE 200

static void handler(int sig) { printf("SIGCHILD handler called\n"); }

int main(int argc, char *argv[])
{
	setbuf(stdout, NULL);

	// Establish SIGCHILD handler
	struct sigaction sa;
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		return EXIT_FAILURE;
	}

	// Block SIGCHILD
	sigset_t block_mask, orig_mask;
	sigemptyset(&block_mask);
	sigaddset(&block_mask, SIGCHLD);
	if (sigprocmask(SIG_BLOCK, &block_mask, &orig_mask) == -1) {
		perror("sigprocmask");
		return EXIT_FAILURE;
	}

	printf("Parent PID=%ld\n", (long)getpid());

	pid_t pid;
	switch (pid = fork()) {
	case -1:
		perror("fork");
		return EXIT_FAILURE;
	case 0:
		printf("Child (PID=%ld) exiting\n", (long)getpid());
		_exit(EXIT_SUCCESS);
	default:
		sleep(1);

		printf("Parent: calling waitpid() while SIGCHILD is blocked\n");

		int status;
		if (waitpid(pid, &status, 0) == -1) {
			perror("waitpid");
			exit(EXIT_FAILURE);
		}

		printf("Parent: child has been reaped\n");

		// Unblock SIGCHILD
		if (sigprocmask(SIG_SETMASK, &orig_mask, NULL) == -1) {
			perror("sigprocmask");
			exit(EXIT_FAILURE);
		}

		printf("Parent: SIGCHILD unblocked\n");

		return EXIT_SUCCESS;
	}
}
