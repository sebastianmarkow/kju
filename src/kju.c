#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 199309L

#include <sys/stat.h>
#include <sys/time.h>
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
#include <unistd.h>

#include "kju.h"

#ifdef PRINT_DEBUG
#define DEBUG(...) fprintf(stdout, __VA_ARGS__)
#else
#define DEBUG(...)
#endif
#define ERROR(...) fprintf(stderr, __VA_ARGS__)

#define PATH_SEPARATOR "/"

static char *KJU_PATH = ".kju";
static char *KJU_PATHENV = "KJUPATH";
static char *KJU_DEFCHAN = "default";

void kju_PrintUsage(void) {
	fprintf(stdout, "Usage: kju\n");
}

void kju_PrintVersion(void) {
	fprintf(stdout, "kju %s (%s)\n", KJU_VERSION, kju_GitSHA1());
}

char *kju_Path(void) {
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

int main(int argc, char **argv) {
	bool qflag = false, vflag = false;
	char *cvalue = NULL;
	char *path = NULL;
	char lock[128];
	int opt = 0;
	int pipefd[2];
	int pathfd, lockfd;
	int64_t ms;
	pid_t cpid;
	struct timeval timestamp;

	openlog("kju", LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	gettimeofday(&timestamp, NULL);
	ms = (int64_t)timestamp.tv_sec * 1000 + timestamp.tv_usec / 1001;

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
			kju_PrintUsage();
			exit(EXIT_SUCCESS);
		default:
			goto usage;
		}
	}
	if (argc < 2) {
	usage:
		kju_PrintUsage();
		exit(EXIT_FAILURE);
	}

	if (vflag) {
		kju_PrintVersion();
		exit(EXIT_SUCCESS);
	}

	path = kju_Path();
	if (!path) {
		ERROR("could not determine kju path");
		exit(EXIT_SUCCESS);
	}

	DEBUG("kju: %s\n", path);

	if (mkdir(path, 0700) < 0) {
		if (errno != EEXIST) {
			perror("mkdir kju path");
			exit(EXIT_FAILURE); // XXX(SK): Error code?
		}
	}

	if (!cvalue) {
		cvalue = KJU_DEFCHAN;
	}
	path = (char *)realloc(path, strlen(cvalue) + 1);
	if (!path) {
		ERROR("could not allocate");
		exit(EXIT_FAILURE); // XXX(SK): Error code? (perror)
	}
	sprintf(path, "%s%s%s", path, PATH_SEPARATOR, cvalue);

	DEBUG("channel: %s\n", path);

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
			fprintf(stdout, "%d\n", pid);
		}

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

		if (write(pipefd[1], &cpid, sizeof(cpid)) < 0) {
			perror("write");
		}
		close(pipefd[1]);

		wait(&cstatus);

		sprintf(lock, "%s%s%011" PRIx64 ".%d", path, PATH_SEPARATOR, ms,
		        cpid);
		lockfd = open(lock, O_RDWR | O_APPEND);
		if (lockfd < 0) {
			syslog(LOG_ERR, "could not open lockfile %s", lock);
			perror("open");
			exit(EXIT_FAILURE); // XXX(SK): error code?
		}

		/* fchmod(lockfd, 0600); */
		/* if (WIFEXITED(cstatus)) { */
		/* 	// TODO(SK): implementation needed */
		/* } else { */
		/* 	// TODO(SK): implementation needed */
		/* } */

		exit(EXIT_SUCCESS);
	}

	close(pipefd[1]);

	sprintf(lock, "%s%s%011" PRIx64 ".%d", path, PATH_SEPARATOR, ms,
	        getpid());
	lockfd = open(lock, O_CREAT | O_EXCL | O_RDWR | O_APPEND, 0600);
	if (lockfd < 0) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
