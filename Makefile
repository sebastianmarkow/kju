release:=$(shell sh -c './scripts/mkrelease.sh')

STD:=-std=c99
WARN:=-Wall -Wextra -Werror -pedantic
OPT:=-O2 -Os

CFLAGS?=$(STD) $(WARN) $(OPT)
LDFLAGS?=

UNAME := $(shell uname)
ifneq ($(UNAME), Darwin)
	LDFLAGS += -lrt
endif

ifeq ($(debug), 1)
	CFLAGS+=-g -ggdb -DPRINT_DEBUG=1
endif

BIN:=kju
SRC:=release.c time.c kju.c
OBJDIR:=_obj
OBJ:=$(patsubst %.c,$(OBJDIR)/%.o,$(SRC))


all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ): $(OBJDIR)

$(OBJDIR):
	@mkdir -p $@

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: check
check:

.PHONY: clean
clean:
	@rm -rf $(BIN)
	@rm -rf $(OBJDIR)

vpath %.c src
vpath %.h src
