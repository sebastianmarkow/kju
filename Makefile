STD=-std=c11
WARN=-Wall -Wextra -pedantic
OPT=-O2

CFLAGS?= $(STD) $(WARN) $(OPT)

all: q

clean:
	rm -rf q
