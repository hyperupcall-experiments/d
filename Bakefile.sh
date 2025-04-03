# shellcheck shell=bash
# shellcheck disable=SC2086

init() {
	CFLAGS='-Wall -Wextra -Wpedantic -g -Wno-unused-variable -Wno-unused-parameter'
}

task.build() {
	bear -- gcc $CFLAGS -DCONFIG_DIR=$1 d.c -o ./d
}

task.run() {
	task.build
	DEBUG=1 ./main "$@"
}
