# com - build with `make`, test with `make test`
CC      ?= cc
CFLAGS  ?= -std=c11 -Wall -Wextra -O2
INC     := -Isrc -Iinclude
PREFIX  ?= /usr/local

LIB_SRC  := $(filter-out src/main.c,$(wildcard src/*/*.c))
LIB_OBJ  := $(patsubst %.c,build/%.o,$(LIB_SRC))
MAIN_OBJ := build/src/main.o

TOOLS    := build/comfmt build/comlint build/comrepl
TESTS    := $(patsubst tests/%.c,build/tests/%,$(wildcard tests/unit/*.c tests/integration/*.c))
BENCHES  := $(patsubst bench/%.c,build/bench/%,$(wildcard bench/*.c))

.PHONY: all tools test bench clean install uninstall

all: build/com tools

tools: $(TOOLS)

build/com: $(MAIN_OBJ) $(LIB_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INC) -MMD -MP -c -o $@ $<

build/comfmt: tools/comfmt/comfmt.c $(LIB_OBJ)
	$(CC) $(CFLAGS) $(INC) -o $@ $^
build/comlint: tools/comlint/comlint.c $(LIB_OBJ)
	$(CC) $(CFLAGS) $(INC) -o $@ $^
build/comrepl: tools/comrepl/comrepl.c $(LIB_OBJ)
	$(CC) $(CFLAGS) $(INC) -o $@ $^

build/tests/%: tests/%.c $(LIB_OBJ)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INC) -Itests -o $@ $^

build/bench/%: bench/%.c $(LIB_OBJ)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INC) -o $@ $^

test: build/com $(TESTS)
	@for t in $(TESTS); do ./$$t || exit 1; done
	@COM=build/com sh tests/run_tests.sh

bench: $(BENCHES)
	@for b in $(BENCHES); do ./$$b; done

clean:
	rm -rf build/*

install: build/com tools
	install -d $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(PREFIX)/share/com/stdlib
	install -m 755 build/com build/comfmt build/comlint build/comrepl $(DESTDIR)$(PREFIX)/bin
	install -m 644 stdlib/*.com $(DESTDIR)$(PREFIX)/share/com/stdlib

uninstall:
	rm -f $(addprefix $(DESTDIR)$(PREFIX)/bin/,com comfmt comlint comrepl)
	rm -rf $(DESTDIR)$(PREFIX)/share/com

-include $(LIB_OBJ:.o=.d) $(MAIN_OBJ:.o=.d)
