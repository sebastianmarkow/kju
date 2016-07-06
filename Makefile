RELEASE:=$(shell sh -c './scripts/mkrelease.sh')

STD:=-std=c99
WARN:=-Wall -Wextra -Werror -pedantic
OPT:=-O2 -Os

CFLAGS?=$(STD) $(WARN) $(OPT)
LDFLAGS?=

PREFIX?=/usr/local
BINDIR?=$(PREFIX)/bin
MANDIR?=$(PREFIX)/share/man
SHAREDIR?=$(PREFIX)/share

BUILDDIR?=build
OBJDIR:=$(BUILDDIR)/_obj

BIN:=kju
SRC:=release.c clock.c kju.c
OBJ:=$(patsubst %.c,$(OBJDIR)/%.o,$(SRC))

UNAME := $(shell uname)
ifneq ($(UNAME), Darwin)
	LDFLAGS += -lrt
endif

ifeq ($(debug), 1)
	CFLAGS+=-g -ggdb -DPRINT_DEBUG=1
endif

.DEFAULT_GOAL: help
.PHONE: help
help:
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "%-20s %s\n", $$1, $$2}' $(MAKEFILE_LIST)

build: $(BUILDDIR)/$(BIN) ## Build binary

install: build ## Install binary
	install -m 0755 $(BUILDDIR)/$(BIN) $(BINDIR)
	install -m 0644 man/$(BIN:=.1) $(MANDIR)/man1

$(BUILDDIR)/$(BIN): $(OBJ)
	@mkdir -p $(BUILDDIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: check
check: ## Run unittests

.PHONY: clean
clean: ## Clean build targets
	rm -rf $(BUILDDIR)

vpath %.c src
vpath %.h src
