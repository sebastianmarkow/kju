#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 199309L

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef PRINT_DEBUG
#define DEBUG(...) fprintf(stdout, __VA_ARGS__)
#else
#define DEBUG(...)
#endif
#define ERROR(...) fprintf(stderr, __VA_ARGS__)

const char *KJU_PATH = ".kju";
const char *KJU_PATH_ENV = "KJUPATH";
const char *HOME_PATH_ENV = "HOME";

struct child_proc {
	pid_t pid;
	bool monitorProc;
	bool killProc;
	int64_t timestamp;
};

static void usage(void) {
	printf("Usage: q\n");
}

char *getKJUPath(void) {
	char *path = malloc(sizeof(char) * 500);
	if (!path) {
		ERROR("could not allocate memory");
		return NULL;
	}

	char *custom_path = getenv(KJU_PATH_ENV);
	if (custom_path) {
		strcpy(path, custom_path);
		DEBUG("return custom"); // XXX(SK)
		return path;
	}

	char *home_path = getenv(HOME_PATH_ENV);
	if (!home_path) {
		ERROR("could not determin home directory"); // XXX(SK)
		return NULL;
	}

	sprintf(path, "%s/%s", home_path, KJU_PATH);
	DEBUG("return default"); // XXX(SK)
	return path;
}

int main(int argc, char **argv) {
	int opt = 0;
	int kjupath_fd = 0;
	int pipe_fd[2];
	struct timeval timestamp;
	struct child_proc *child;

	gettimeofday(&timestamp, NULL);

	while ((opt = getopt(argc, argv, "")) != -1) {
		switch (opt) {
		default:
			goto usage;
		}
	}

	if (argc < 2) {
	usage:
		usage();
		return 1;
	}

	char *kjupath = getKJUPath();
	if (!kjupath) {
		ERROR("Could not determin queue path");
		return 1;
	}

	printf("path: %s\n", kjupath);

	if (mkdir(kjupath, 0700) < 0) {
		if (errno != EEXIST) {
			perror("Could not create queue path");
			return 200; // XXX(SK): Error code?
		}
	}

	kjupath_fd = open(kjupath, O_RDONLY);
	if (kjupath_fd < 0) {
		perror("Could not open queue path");
		return 200; // XXX(SK): Error code?
	}

	free(kjupath);
	kjupath = NULL;

	if (pipe(pipe_fd) < 0) {
		perror("Could not init pipe");
		return 200; // XXX(SK): Error code?
	}

	pid_t pid = fork();

	if (pid > 0) {
		child = malloc(sizeof(struct child_proc));
		child->pid = pid;
		child->timestamp = (int64_t)timestamp.tv_sec*1000 + timestamp.tv_usec/1000;


		printf("child pid: %d\n", child->pid);
		printf("child timestamp: %ld\n", (long) child->timestamp);

		sleep(100);

		free(child);
		child = NULL;
	} else {
		printf("parent pid: %d\n", pid);
		sleep(100);
	}

	close(kjupath_fd);

	return 0;
}
