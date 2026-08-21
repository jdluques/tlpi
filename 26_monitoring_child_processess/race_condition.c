#include <errno.h>
#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define CMD_SIZE 200

static void handler(int sig) {}

int main(int argc, char *argv[])
{
	char cmd[CMD_SIZE];
	pid_t child_pid;

	setbuf(stdout, NULL);

	sigset_t block_mask, orig_mask, empty_mask;
	struct sigaction sa;

	sigemptyset(&block_mask);
	sigaddset(&block_mask, SIGCHLD);
	if (sigprocmask(SIG_BLOCK, &block_mask, &orig_mask) == -1) {
		perror("sigprocmask");
		return EXIT_FAILURE;
	}

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = handler;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		return EXIT_FAILURE;
	}

	printf("Parent PID=%ld\n", (long)getpid());

	switch (child_pid = fork()) {
	case -1:
		perror("fork");
		return EXIT_FAILURE;
	case 0:
		printf("Child (PID=%ld) exiting\n", (long)getpid());
		_exit(EXIT_SUCCESS);
	default:
		sigemptyset(&empty_mask);
		if (sigsuspend(&empty_mask) == -1 && errno != EINTR) {
			perror("sigsuspend");
			exit(EXIT_FAILURE);
		}

		if (sigprocmask(SIG_SETMASK, &orig_mask, NULL) == -1) {
			perror("sigprocmask");
			exit(EXIT_FAILURE);
		}

		snprintf(cmd, CMD_SIZE, "ps | grep %s", basename(argv[0]));
		cmd[CMD_SIZE - 1] = '\0';
		system(cmd);

		if (kill(child_pid, SIGKILL) == -1) {
			perror("kill");
		}
		sleep(3);
		printf("After sending SIGKILL to zombie (PID=%ld):\n", (long)child_pid);
		system(cmd);

		exit(EXIT_SUCCESS);
	}
}
