#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <linux/limits.h>
#include <pwd.h>

#include "util.h"

int main(int argc, char *argv[]) {
	struct Options options = generate_options(argc, argv);
	struct Config config = parse_config(options);

	if (options.command == COMMAND_INIT) {
		command_init(options, config);
	} else if (options.command == COMMAND_LIST) {
		command_list(options, config);
	} else if (options.command == COMMAND_DEPLOY) {
		command_deploy(options, config);
	} else if (options.command == COMMAND_UNDEPLOY) {
		command_undeploy(options, config);
	} else if (options.command == COMMAND_REPLACE) {
		command_replace(options, config);
	} else {
		die("Failed to recognize subcommand: %i", options.command);
	}
}
