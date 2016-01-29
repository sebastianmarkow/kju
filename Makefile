STD:=-std=c99
WARN:=-Wall -Wextra -Werror -pedantic
OPT:=-O2 -Os

CFLAGS?=$(STD) $(WARN) $(OPT)
LDFLAGS?=

ifeq ($(debug), 1)
	CFLAGS+=-g -ggdb -DPRINT_DEBUG=1
endif

BIN:=kju
SRC:=kju.c release.c
OBJDIR:=_obj
OBJ:=$(patsubst %.c,$(OBJDIR)/%.o,$(SRC))

release:=$(shell sh -c './scripts/mkrelease.sh')


all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

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
