#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define Q_DEFAULT_PATH ".q"
#define Q_PATH_ENV "QPATH"
#define HOME_ENV "HOME"


#ifdef PRINT_DEBUG
#define DEBUG(...) fprintf(stdout, __VA_ARGS__)
#else
#define DEBUG(...)
#endif
#define ERROR(...) fprintf(stderr, __VA_ARGS__)

static void usage(void) {
	printf("Usage: q\n");
}

char *getQPath(void) {
	char *path = getenv(Q_PATH_ENV);
	if (path) {
		DEBUG("test")
		return path;
	}

	char *home_path = getenv(HOME_ENV);
	if (!home_path) {
		return path;
	}

	path = (char *)realloc(path, sizeof(char) * 500);
	snprintf(path, 500, "%s/%s", home_path, Q_DEFAULT_PATH);

	return path;
}

int main(int argc, char **argv) {
	int opt = 0;
	int qpath_fd = 0;

	while ((opt = getopt(argc, argv, "")) != -1) {
		switch (opt) {
		default:
			goto usage;
		}
	}

	if (argc < 2) {
	usage:
		usage();
		exit(1);
	}

	char *qpath = getQPath();
	if (!qpath) {
		perror("Could not determin queue path");
		exit(1);
	}

	if (mkdir(qpath, 0700) < 0) {
		if (errno != EEXIST) {
			perror("Could not create queue path");
			exit(200); // XXX(SK): Error code?
		}
	}

	qpath_fd = open(qpath, O_RDONLY);
	if (qpath_fd < 0) {
		perror("Could not open queue path");
		exit(200); // XXX(SK): Error code?
	}

	printf("%s\n", qpath);

	return 0;
}
