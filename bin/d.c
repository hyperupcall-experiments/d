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
	struct Cli cli = parse_cli(argc, argv);
	if (cli.help) {
		show_help();
		exit(0);
	}
	struct Config config = parse_config(cli.config_file);

	if (STR_EQ(cli.command, "init")) {
		command_init(cli, config);
	} else if (STR_EQ(cli.command, "list")) {
		command_list(cli, config);
	} else if (STR_EQ(cli.command, "deploy")) {
		command_deploy(cli, config);
	} else if (STR_EQ(cli.command, "replace")) {
		command_replace(cli, config);
	} else if (STR_EQ(cli.command, "undeploy")) {
		command_undeploy(cli, config);
	} else {
		die("Command not accounted for");
	}
}
