#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>

#include "../src/util.h"

static void rmrf(char *path) {
	char buf[PATH_MAX];
	sprintf("rm -rf %s", path);

	system(buf);
}

static void run(char *test_dirname) {
	// Setup
	char config_file[PATH_MAX];
	sprintf(config_file, "../testdata/%s/d.conf", test_dirname);

	// Test
	rmrf(config_file);
	struct Options options = {
		.command = COMMAND_DEPLOY,
		.config_file = config_file,
		.dry_run = false,
		.interactive = false,
		.log = LOG_NORMAL,
		.help = false,
	};
	struct Config config = parse_config(options);
	command_deploy(options, config);

	// Teardown
	rmrf(config_file);
}

int main() {
	run("10-deploy-file");
}
