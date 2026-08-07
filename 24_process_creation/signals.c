#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUF_SIZE 1000

volatile int got_sigusr1 = 0;
volatile sig_atomic_t last_signal = 0;

static void handler(int sig)
{
	got_sigusr1 = 1;
	last_signal = sig;
}

char *curr_time(const char *format)
{
	static char buf[BUF_SIZE];

	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	if (tm == NULL) {
		return NULL;
	}

	size_t s = strftime(buf, BUF_SIZE, (format != NULL) ? format : "%c", tm);

	return (s == 0) ? NULL : buf;
}

int main()
{
	setbuf(stdout, NULL);

	sigset_t block_mask, orig_mask, emtpy_mask;

	// Create signal mask with SIGUSR1 in block_mask
	sigemptyset(&block_mask);
	sigaddset(&block_mask, SIGUSR1);

	// Block signals in block_mask (SIGUSR1) and store previous signal mask in orig_mask
	if (sigprocmask(SIG_BLOCK, &block_mask, &orig_mask) == -1) {
		perror("sigprocmask");
		return EXIT_FAILURE;
	}

	// Struct that defines how a signal is handled
	struct sigaction sa;
	// While handler is running don't block any additional signals
	sigemptyset(&sa.sa_mask);
	// If a signal interrupts while a syscall is running, it will restart it after the handler
	// is finished
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = handler;
	// Register action for SIGUSR1 signal, previous sigaction is not stored
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		perror("sigaction");
		return EXIT_FAILURE;
	}

	pid_t child_pid = fork();

	switch (child_pid) {
	case -1:
		perror("fork");
		return EXIT_FAILURE;

	case 0:
		printf("[%s %ld] Child about to wait for signal\n", curr_time("%T"), (long)getpid());
		sigemptyset(&emtpy_mask);
		// Wait until SIGUSR1 arrives
		while (!got_sigusr1) {
			if (sigsuspend(&emtpy_mask) == -1 && errno != EINTR) {
				perror("sigsuspend");
				_exit(EXIT_FAILURE);
			}
		}

		printf("[%s %ld] Child got signal %d\n", curr_time("%T"), (long)getpid(), (int)last_signal);

		printf("[%s %ld] Child started - doing some work\n", curr_time("%T"), (long)getpid());
		sleep(2);

		printf("[%s %ld] Child about to signal parent\n", curr_time("%T"), (long)getpid());
		// Send SIGUSR1 signal to parent
		if (kill(getppid(), SIGUSR1) == -1) {
			perror("kill");
			_exit(EXIT_FAILURE);
		}

		_exit(EXIT_SUCCESS);

	default:
		printf("[%s %ld] Parent started - doing some work\n", curr_time("%T"), (long)getpid());
		sleep(2);

		printf("[%s %ld] Parent about to signal child\n", curr_time("%T"), (long)getpid());
		// Send SIGUSR1 signal to child
		if (kill(child_pid, SIGUSR1) == -1) {
			perror("kill");
			_exit(EXIT_FAILURE);
		}

		sleep(2);
		printf("[%s %ld] Parent about to wait for signal\n", curr_time("%T"), (long)getpid());
		sigemptyset(&emtpy_mask);
		// Wait until SIGUSR1 arrives
		while (!got_sigusr1) {
			// Wait for any signal (since emtpy_mask is empty so no signal is blocked) to arrive
			// If it receives a signal succesfully it returns -1 and sets errno to EINTR
			if (sigsuspend(&emtpy_mask) == -1 && errno != EINTR) {
				perror("sigsuspend");
				return EXIT_FAILURE;
			}
		}

		printf("[%s %ld] Parent got signal %i\n", curr_time("%T"), (long)getpid(),
			   (int)last_signal);
		// Restore original signal mask
		if (sigprocmask(SIG_SETMASK, &orig_mask, NULL) == -1) {
			perror("sigprocmask");
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}
}
