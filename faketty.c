#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *const tty_env_vars[3] = {
	[STDIN_FILENO] = "FAKETTY_STDIN_ISATTY__",
	[STDOUT_FILENO] = "FAKETTY_STDOUT_ISATTY__",
	[STDERR_FILENO] = "FAKETTY_STDERR_ISATTY__"
};

static void print_usage(const char *progname, FILE *to)
{
	fprintf(to, "Usage: %s [-iIoOeEhv] [--] COMMAND ARGS...\n", progname);
}

static void print_help(const char *progname, FILE *to)
{
	print_usage(progname, to);
	fputs(
"Run COMMAND with ARGS, tricking it into believing certain streams are ttys\n"
"intercepting isatty calls. Be default, these calls are left alone.\n"
"\n"
"Options:\n"
"  -i  Pretend standard input is a tty.\n"
"  -I  Pretend standard input is not a tty.\n"
"  -o  Pretend standard output is a tty.\n"
"  -O  Pretend standard output is not a tty.\n"
"  -e  Pretend standard error is a tty.\n"
"  -E  Pretend standard error is not a tty.\n"
"  -h  Print this help information and exit.\n"
"  -v  Print version information and exit.\n"
	, to);
}

static void print_version(const char *progname, FILE *to)
{
	fprintf(to, "%s version "VERSION"\n", progname);
}

int main(int argc, char *argv[])
{
	char preload[PATH_MAX];
	char *posixly_correct;
	int opt;
	unsetenv(tty_env_vars[STDIN_FILENO]);
	unsetenv(tty_env_vars[STDOUT_FILENO]);
	unsetenv(tty_env_vars[STDERR_FILENO]);
	if (readlink("/proc/self/exe", preload, sizeof(preload)) < 0) {
		fprintf(stderr, "%s: Failed to read link /proc/self/exe; %s\n",
			argv[0], strerror(errno));
		return 1;
	}
	setenv("LD_PRELOAD", preload, 1);
	/* Stupid GNU getopt looks for options after the first non-option, which
	 * messes with e.g. 'faketty -O ls -alt'. Setting POSIXLY_CORRECT
	 * disables that behaviour: */
	posixly_correct = getenv("POSIXLY_CORRECT");
	setenv("POSIXLY_CORRECT", "1", 1);
	while ((opt = getopt(argc, argv, "iIoOeEhv")) >= 0) {
		switch (opt) {
		case 'i':
			setenv(tty_env_vars[STDIN_FILENO], "1", 1);
			break;
		case 'I':
			setenv(tty_env_vars[STDIN_FILENO], "0", 1);
			break;
		case 'o':
			setenv(tty_env_vars[STDOUT_FILENO], "1", 1);
			break;
		case 'O':
			setenv(tty_env_vars[STDOUT_FILENO], "0", 1);
			break;
		case 'e':
			setenv(tty_env_vars[STDERR_FILENO], "1", 1);
			break;
		case 'E':
			setenv(tty_env_vars[STDERR_FILENO], "0", 1);
			break;
		case 'h':
			print_help(argv[0], stdout);
			return 1;
		case 'v':
			print_version(argv[0], stdout);
			return 0;
		default:
			print_usage(argv[0], stderr);
			return 1;
		}
	}
	/* Reset POSIXLY_CORRECT without a trace: */
	if (posixly_correct) {
		setenv("POSIXLY_CORRECT", posixly_correct, 1);
	} else {
		unsetenv("POSIXLY_CORRECT");
	}
	if (argv[optind]) {
		/* Command was provided to run. */
		int i;
		execvp(argv[optind], argv + optind);
		/* An error occurred. */
		fprintf(stderr, "%s: %s; Could not run command",
			argv[0], strerror(errno));
		for (i = optind; i < argc; ++i) {
			fprintf(stderr, " %s", argv[i]);
		}
		fputc('\n', stderr);
	} else {
		print_usage(argv[0], stderr);
	}
	return 1;
}

/* Dynamic linking stuff */

static char isatty_is[3];
static char isatty_fake[3];

__attribute__((constructor))
static void init(void)
{
	int fd;
	for (fd = 0; fd < 3; ++fd) {
		char *env = getenv(tty_env_vars[fd]);
		if (env) {
			isatty_fake[fd] = 1;
			isatty_is[fd] = *env == '1';
		}
	}
}

int isatty(int fd)
{
	if ((fd == STDIN_FILENO || fd == STDOUT_FILENO || fd == STDERR_FILENO)
	 && isatty_fake[fd]) {
		if (isatty_is[fd]) return 1;
	} else {
		int (*orig_isatty)(int fd) = dlsym(RTLD_NEXT, "isatty");
		if (orig_isatty) return orig_isatty(fd);
	}
	errno = EINVAL;
	return 0;
}
