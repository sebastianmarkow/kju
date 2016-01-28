#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#define QPATH ".q"
#define QPATHENV "QPATH"

static void usage(void) {
	printf("Usage: q\n");
}

char *getQueuePath(void) {
	char *path = getenv(QPATHENV);
	if (path) {
		return path;
	}

	char *home = getenv("HOME");
	if (!home) {
		return path;
	}

	path = (char *)realloc(path, sizeof(char) * 500);
	snprintf(path, 500, "%s/%s", home, QPATH);

	return path;
}

int main(int argc, char **argv) {
	int opt = 0;

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

	char *q_path = getQueuePath();
	if (!q_path) {
		printf("Error: \n");
	}

	printf("%s\n", q_path);

	return 0;
}
