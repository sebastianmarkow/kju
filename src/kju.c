#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "kju.h"

#include "debug.h"
#include "clock.h"

#define PATH_SEPARATOR "/"

static char *KJU_PATH = ".kju";
static char *KJU_PATHENV = "KJUPATH";
static char *KJU_DEFCHAN = "default";

void print_usage(void) { fprintf(stdout, "Usage: kju\n"); }

void print_version(void)
{
	fprintf(stdout, "kju %s (build %s)\n", KJU_VERSION, kju_gitsha1());
}

char *kju_path(void)
{
	char *ppath, *hpath;
	static char *path = NULL;

	ppath = getenv(KJU_PATHENV); // custom kju path
	hpath = getenv("HOME");      // home directory

	switch (!ppath << 1 | !hpath << 0) {
	case 0:
	// fall-through
	case 1:
		// use custom kju path
		path = (char *)realloc(path, strlen(ppath) + 1);
		if (!path) {
			return path;
		}
		strcpy(path, ppath);
		break;
	case 2:
		// use default path in home directory
		path =
		    (char *)realloc(path, strlen(hpath) + strlen(KJU_PATH) + 2);
		if (!path) {
			return path;
		}
		sprintf(path, "%s%s%s", hpath, PATH_SEPARATOR, KJU_PATH);
		break;
	}

	return path;
}

int main(int argc, char **argv)
{
	bool qflag = false, vflag = false;
	char *cvalue = NULL;
	char *path = NULL;
	char lock[128];
	int opt = 0;
	int pathfd, lockfd;
	int pipefd[2];
	int64_t ns;
	pid_t cpid;
	struct flock fl;
	struct timespec timestamp;

	DEBUG("%s\n", "YEAH!");

	openlog("kju", LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	clock_gettime(CLOCK_MONOTONIC, &timestamp);
	ns = (int64_t)(timestamp.tv_sec * 1000000 + timestamp.tv_nsec);

	while ((opt = getopt(argc, argv, "+c:hqv")) != -1) {
		switch (opt) {
		case 'c':
			// TODO(SK): alphanumeric only
			cvalue = optarg;
			break;
		case 'q':
			qflag = true;
			break;
		case 'v':
			vflag = true;
			break;
		case 'h':
			print_usage();
			exit(EXIT_SUCCESS);
		default:
			goto usage;
		}
	}
	if (argc < 2) {
	usage:
		print_usage();
		exit(EXIT_FAILURE);
	}

	if (vflag) {
		print_version();
		exit(EXIT_SUCCESS);
	}

	if (!cvalue) {
		cvalue = KJU_DEFCHAN;
	}

	path = kju_path();
	if (!path) {
		exit(EXIT_SUCCESS);
	}

	if (mkdir(path, 0700) < 0) {
		if (errno != EEXIST) {
			perror("mkdir kju path");
			exit(EXIT_FAILURE); // XXX(SK): Error code?
		}
	}

	path = (char *)realloc(path, strlen(path) + strlen(cvalue) + 2);
	if (!path) {
		exit(EXIT_FAILURE); // XXX(SK): Error code? (perror)
	}
	sprintf(path, "%s%s%s", path, PATH_SEPARATOR, cvalue);

	if (mkdir(path, 0700) < 0) {
		if (errno != EEXIST) {
			perror("mkdir channel");
			exit(EXIT_FAILURE); // XXX(SK): Error code?
		}
	}

	if ((pathfd = open(path, O_RDONLY)) < 0) {
		perror("dir open");
		exit(111);
	}

	if (pipe(pipefd) < 0) {
		perror("pipe");
		exit(EXIT_FAILURE); // XXX(SK): Error code?
	}

	if ((cpid = fork()) < 0) {
		perror("fork");
		exit(EXIT_FAILURE); // XXX(SK): Error code?
	} else if (cpid > 0) {
		pid_t pid = -1;

		close(pipefd[1]);
		if (read(pipefd[0], &pid, sizeof(pid)) < 0) {
			perror("read");
		}

		if (!qflag) {
			char pid_str[32];
			sprintf(pid_str, "%d\n", pid);
			if (write(1, pid_str, sizeof(pid_str)) < 0) {
				perror("write");
				exit(EXIT_FAILURE);
			}
		}

		close(0);
		close(1);
		close(2);

		exit(EXIT_SUCCESS);
	}

	close(pipefd[0]);

	if ((cpid = fork()) < 0) {
		perror("fork");
		exit(EXIT_FAILURE); // XXX(SK): Error code?
	} else if (cpid > 0) {
		int cstatus;

		close(0);
		close(1);
		close(2);

		close(pipefd[1]);

		wait(&cstatus);

		sprintf(lock, "%s%s%011" PRIx64 ".%d", path, PATH_SEPARATOR, ns,
			cpid);
		lockfd = open(lock, O_RDWR | O_APPEND);
		if (lockfd < 0) {
			syslog(LOG_ERR, "could not open lockfile %s", lock);
			// TODO(SK): perror to char*
			exit(EXIT_FAILURE); // XXX(SK): error code?
		}

		// TODO(SK): write exit code

		exit(EXIT_SUCCESS);
	}

	sprintf(lock, "%s%s.%011" PRIx64 ".%d", path, PATH_SEPARATOR, ns,
		getpid());

	lockfd = open(lock, O_CREAT | O_EXCL | O_RDWR | O_APPEND, 0600);
	if (lockfd < 0) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	pid_t ppid = getpid();

	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;
	fl.l_len = ppid;

	if (fcntl(lockfd, F_SETLK, &fl) < 0) {
		perror("fcntl");
		exit(EXIT_FAILURE);
	}

	char *newlock = NULL;
	sprintf(newlock, "%s%s%011" PRIx64 ".%d", path, PATH_SEPARATOR, ns,
		getpid());
	rename(lock, newlock);

	fsync(pathfd);

	if (dup2(lockfd, 2) < 0 || dup2(lockfd, 1) < 0) {
		perror("dup2");
		exit(222);
	}

	if (write(pipefd[1], &ppid, sizeof(ppid)) < 0) {
		perror("write");
		exit(EXIT_FAILURE);
	}
	close(pipefd[1]);

	exit(EXIT_SUCCESS);
}
