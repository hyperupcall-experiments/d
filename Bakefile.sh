# shellcheck shell=bash
# shellcheck disable=SC2086

init() {
	CFLAGS='-Wall -Wextra -Wpedantic -g -Wno-unused-variable -Wno-unused-parameter'
}

task.install() {
	make CONFIG_FILE="$HOME/.dotfiles/os-unix/data/dotfiles.h" install
}

task.build() {
	gcc $CFLAGS d.c -o ./d
}

task.run() {
	task.build
	DEBUG=1 ./main "$@"
}
