#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	switch (fork()) {
	case -1:
		perror("fork");
		return EXIT_FAILURE;
	case 0:
		printf("Child started with PID = %ld\n", (long)getpid());
		if (argc > 1) {
			exit(atoi(argv[1]));
		} else {
			for (;;) {
				pause();
			}
		}
		exit(EXIT_FAILURE);
	default:
		for (;;) {
			siginfo_t siginfo;

			if (waitid(P_ALL, 0, &siginfo, WEXITED) == -1) {
				perror("waitid");
				return EXIT_FAILURE;
			}

			pid_t child_pid = siginfo.si_pid;
			int status = siginfo.si_status;
			int code = siginfo.si_code;

			printf("waitid() returned: PID=%ld; code=%d; status=%d; UID=%ld; signo=%d\n",
				   (long)child_pid, code, status, (long)siginfo.si_uid, siginfo.si_signo);

			if (code == CLD_EXITED) {
				printf("child exited, status=%d\n", status);

			} else if (code == CLD_KILLED) {
				printf("child killed by signal %d (%s)\n", status, strsignal(status));
			} else if (code == CLD_DUMPED) {
				printf("child killed by signal %d (%s) (core dumped)\n", status, strsignal(status));
			}

			if (code == CLD_EXITED || code == CLD_KILLED || code == CLD_DUMPED) {
				exit(EXIT_SUCCESS);
			}
		}
	}
}
