.PHONY: all test

all: test

test: test/hash.exe
	test/hash.exe

debug: test/hash.exe
	gdb --args $<

test/hash.exe: test/hash.c hash.h
	clang $< -O3 -ggdb -o $@ -lpthread -mcx16
