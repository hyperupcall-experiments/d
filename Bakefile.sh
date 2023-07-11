# shellcheck shell=bash

# shellcheck disable=SC2086

init() {
	export LD_LIBRARY_PATH=./inih/build
	CFLAGS='-Wall -Wextra -Wpedantic -g -Wno-unused-variable -Wno-unused-parameter'
}

task.build() {
    gcc -L ./inih/build $CFLAGS main.c -DIS_NOT_TEST -linih -o ./main
}

task.run() {
	task.build
	DEBUG=1 ./main "$@"
}

task.test() {
	gcc -L ./inih/build $CFLAGS main.c test.c -linih -o ./test \
		&& DEBUG=1 ./test "$@"
}
