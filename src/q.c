#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void usage(void) {
	printf("Usage: q\n");
}

int main(int argc, char **argv) {
	int opt = 0;

	while ((opt = getopt(argc, argv, "")) != -1) {
		switch (opt) {
		default:
			goto usage;

		}
	}

	if (argc <= 1) {
usage:
		usage();
		exit(1);
	}


	return 0;
}
