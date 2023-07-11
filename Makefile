LD_LIBRARY_PATH := ./inih/build

.PHONY: run
run: build
	./main

.PHONY: build
build:
	gcc -L ./inih/build -Wall -Wextra -Wpedantic main.c -DIS_NOT_TEST -linih -o ./ma

.PHONY: test
test: test-build
	./test

.PHONY: test-build
test-build:
	echo building test
