RELEASE:=$(shell sh -c './scripts/mkrelease.sh')

STD:=-std=c99
WARN:=-Wall -Wextra -Werror -pedantic
OPT:=-O2 -Os

CFLAGS?=$(STD) $(WARN) $(OPT) -flto
LDFLAGS?=-flto

PREFIX?=/usr/local
BINDIR?=$(PREFIX)/bin
MANDIR=$(PREFIX)/share/man
SHAREDIR?=$(PREFIX)/share

BIN:=kju
SRC:=release.c clock.c kju.c
OBJDIR:=_obj
OBJ:=$(patsubst %.c,$(OBJDIR)/%.o,$(SRC))

UNAME := $(shell uname)
ifneq ($(UNAME), Darwin)
	LDFLAGS += -lrt
endif

ifeq ($(debug), 1)
	CFLAGS+=-g -ggdb -DPRINT_DEBUG=1
endif


all: $(BIN)

install: all
	install -m 0755 $(BIN) $(BINDIR)
	install -m 0644 man/$(BIN:=.1) $(MANDIR)/man1

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
