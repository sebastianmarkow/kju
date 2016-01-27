STD?=-std=c99
WARN?=-Wall -Wextra -pedantic
OPT?=-O2

CFLAGS?=$(STD) $(WARN) $(OPT)
LDFLAGS?=

BIN:=q
SRC:=q.c release.c
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
