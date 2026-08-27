#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

int system(const char *command)
{
	if (command == NULL) {
		return system(":") == 0; // Is a shell available?
	}

	// Block SIGCHILD temporarily to prevent caller's SIGCHILD handler to handle the shell child
	sigset_t block_mask, orig_mask;
	sigemptyset(&block_mask);
	sigaddset(&block_mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &block_mask, &orig_mask);

	// Ignore SIGINT and SIGQUIT
	struct sigaction sa_ignore, sa_orig_int, sa_orig_quit;
	sa_ignore.sa_handler = SIG_IGN;
	sa_ignore.sa_flags = 0;
	sigemptyset(&sa_ignore.sa_mask);
	sigaction(SIGINT, &sa_ignore, &sa_orig_int);
	sigaction(SIGQUIT, &sa_ignore, &sa_orig_quit);

	pid_t child_pid;
	int status;
	switch (child_pid = fork()) {
	case -1:
		status = -1;
		break;

	case 0: {
		struct sigaction sa_default;
		sa_default.sa_handler = SIG_DFL;
		sa_default.sa_flags = 0;
		sigemptyset(&sa_default.sa_mask);

		// If SIGINT was not ignored previously then set to default
		if (sa_orig_int.sa_handler != SIG_IGN) {
			sigaction(SIGINT, &sa_default, NULL);
		}
		// If SIGQUIT was not ignored previously then set to default
		if (sa_orig_quit.sa_handler != SIG_IGN) {
			sigaction(SIGQUIT, &sa_default, NULL);
		}

		// Unblock SIGCHILD in shell child
		sigprocmask(SIG_SETMASK, &orig_mask, NULL);

		// Replace process by shell and execute command
		execl("/bin/sh", "sh", "-c", command, (char *)NULL);

		// If call to execl failed exit with 127
		_exit(127);
	}
	default:
		while (waitpid(child_pid, &status, 0) == -1) {
			if (errno != EINTR) {
				status = -1;
				break;
			}
		}
		break;
	}

	// Save errno
	int saved_errno = errno;

	// Unblock SIGCHILD and restore SIGINT and SIGQUIT handlers
	sigprocmask(SIG_SETMASK, &orig_mask, NULL);
	sigaction(SIGINT, &sa_orig_int, NULL);
	sigaction(SIGQUIT, &sa_orig_quit, NULL);

	// Restore errno
	errno = saved_errno;

	return status;
}
