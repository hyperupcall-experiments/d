#ifdef __unix__
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
#else
#error "Unsupported platform"
#endif

#include "ini.h"
#include "util.h"

/** PLUMBING UTILITY FUNCTIONS */
char *compat_getenv(char *var) {
	char *s;

#ifdef __unix__
	s = getenv(var);
#else
	die("Not implemented: compat_getenv");
#endif

	return s;
}

/** APPLICATION HELPER FUNCTIONS */
void die(char *message) {
	fprintf(stderr, "Error: %s\n", message);
	exit(1);
}

bool should_debug() {
	char *s = compat_getenv("DEBUG");
	return s != NULL;
}

void show_help() {
	char *s = "Usage: d [subcommand] [options]\n"
	           "\n"
	           "Subcommands:\n"
	           "list\n"
	           "  list the currently configured files"
	           "deploy\n"
	           "replace\n"
	           "undeploy\n"
	           "\n"
	           "Options:\n"
	           "--config-file\n"
	           "--dry-run\n"
	           "--interactive\n"
	           "--log=<logLevel>\n"
	           "--help\n";
	printf("%s", s);
}

/** APPLICATION UTILITY FUNCTIONS */
void deploy_symlink(struct Entry entry) {
#ifdef __linux__
	// symlink(source, target);
#endif
}



void command_init(struct Cli cli, struct Config config) {
	printf("%s\n", "init");
}

void command_list(struct Cli cli, struct Config config) {
	printf("%s\n", "init");
}

void command_deploy(struct Cli cli, struct Config config) {
	printf("%s\n", "deploy");
}

void command_replace(struct Cli cli, struct Config config) {
	printf("%s\n", "replace");
}

void command_undeploy(struct Cli cli, struct Config config) {
	printf("%s\n", "undeploy");
}

struct ExpandStringResult expand_string(char *input, struct ExpandStringVars vars) {
	struct ExpandStringResult result;

	char *output = malloc(200);
	memset(output, 0, 200);
	result.str = output;

	char *start = input;

	while (true) {
		char *dollar = strchr(start, '$');
		if (dollar == NULL) {
			strcat(output, start);
			result.code = ES_SUCCESS;
			break;
		} else {
			char *left = strchr(start, '{');
			while(true) {
				if (left == NULL) {
					result.code = ES_FAILURE_DOLLAR_NO_FOLLOWED;
					goto done;
				} else if (left < dollar) {
					if (*(left - 1) != '\\') {
						result.code = ES_FAILURE_ESCAPE;
						goto done;
					} else {
						left = strchr(left + 1, '{');
					}
				} else {
					break;
				}
			}

			char *right = strchr(start, '}');
			while(true) {
				if (right == NULL) {
					result.code = ES_FAILURE_DOLLAR_NO_FOLLOWED;
					goto done;
				} else if (right < dollar) {
					if (*(right - 1) != '\\') {
						result.code = ES_FAILURE_ESCAPE;
						goto done;
					} else {
						right = strchr(right + 1, '}');
					}
				} else {
					break;
				}
			}

			if (dollar + 1 != left) {
				result.code = ES_FAILURE_DOLLAR_NO_FOLLOWED;
				goto done;
			}

			if (left == right - 1) {
				result.code = ES_INVALID_VARIABLE;
				goto done;
			}

			for (char *ptr = left + 1; ptr < right; ++ptr) {
				if (!isalpha(*ptr)) {
					printf("ptr: %c\n", *ptr);
					result.code = ES_INVALID_VARIABLE;
					goto done;
				}
			}
			strncat(output, start, dollar - start);

			char *var = malloc(right - left);
			memset(var, 0, right - left);
			strncat(var, left + 1, right - left - 1);
			bool found = false;
			for (unsigned int i = 0; i < vars.len; ++i) {
				if (STR_EQ(var, vars.vars[i].key)) {
					strcat(output, vars.vars[i].value);
					result.code = ES_SUCCESS;
					found = true;
				}
			}

			if (!found) {
				result.code = ES_INVALID_VARIABLE;
				goto done;
			}

			start = right + 1;
		}
	}
	done:

	return result;
}

int parse_config_handler(void *data, const char *section, const char *key, const char *value) {
	struct Config *config = (struct Config *)data;

	bool is_debug = should_debug();
	if (is_debug) {
		fprintf(stderr, "DEBUG: section: %s\n", section);
		fprintf(stderr, "       key: %s\n", key);
		fprintf(stderr, "       value: %s\n", value);
	}

char pwddir[PATH_MAX];
		if (getcwd(pwddir, sizeof(pwddir)) == NULL) {
			die("Failed to get cwd");
		}

		char *homedir;
		if ((homedir = getenv("HOME")) == NULL) {
			homedir = getpwuid(getuid())->pw_dir;
		}
		if (homedir == NULL) {
			die("homedir null");
		}

		char *cfgdir = malloc(strlen(homedir) + strlen("/.config") + 1);
		strcat(cfgdir, homedir);
		strcat(cfgdir, "/.config");

		struct ExpandStringVars vars = {
			.len = 3,
			.vars = {
				{.key = "home", .value = homedir},
				{.key = "pwd", .value = pwddir},
				{.key = "cfg", .value = cfgdir},
			},
		};

	if (STR_EQ(section, "")) {

		if (STR_EQ(key, "source_root_dir")) {
			char* ptr = malloc(strlen(value) + 1);
			strcpy(ptr, value);

			struct ExpandStringResult result = expand_string(ptr, vars);
			if (result.code != ES_SUCCESS) {
				die("Failed to expand string 1");
			}
			config->source_root_dir = result.str;
		} else if (STR_EQ(key, "target_root_dir")) {
			char* ptr = malloc(strlen(value) + 1);
			strcpy(ptr, value);
			struct ExpandStringResult result = expand_string(ptr, vars);
			if (result.code != ES_SUCCESS) {
				die("Failed to expand string 2");
			}
			config->target_root_dir = result.str;
		} else {
			die("Unknown key under root");
		}
	} else if (STR_EQ(section, "vars")) {
		fprintf(stderr, "Skipping key: %s\n", key);
		die("not implemented");
	} else if (STR_EQ(section, "entry")) {
		if (STR_EQ(key, "path")) {
			if (config->current_entry_ready) {
				die("Forgot to add what");
			}
			config->current_entry_ready = false;

			char* s = malloc(strlen(value) + 1);
			memcpy(s, value, strlen(value) + 1);

			struct ExpandStringResult result = expand_string(s, vars);
			if (result.code != ES_SUCCESS) {
				die("Failed to expand string 3");
			}
			config->current_entry.path = result.str;
		} else if (STR_EQ(key, "source")) {
			if (config->current_entry_ready) {
				die("Forgot to add what");
			}
			config->current_entry_ready = false;

			char* s = malloc(strlen(value) + 1);
			memcpy(s, value, strlen(value) + 1);
			printf("aa s: %s\n", s);
			struct ExpandStringResult result = expand_string(s, vars);
			if (result.code != ES_SUCCESS) {
				die("Failed to expand string 4");
			}
			config->current_entry.source = result.str;
		} else if (STR_EQ(key, "target")) {
			if (config->current_entry_ready) {
				die("Forgot to add what");
			}
			config->current_entry_ready = false;

		char* s = malloc(strlen(value) + 1);
		memcpy(s, value, strlen(value) + 1);
		struct ExpandStringResult result = expand_string(s, vars);
			if (result.code != ES_SUCCESS) {
				die("Failed to expand string 5");
			}
			config->current_entry.target = result.str;
		} else if (STR_EQ(key, "what")) {
			config->current_entry_ready = true;

			char* s = malloc(strlen(value) + 1);
			memcpy(s, value, strlen(value) + 1);
			struct ExpandStringResult result = expand_string(s, vars);
			if (result.code != ES_SUCCESS) {
				die("Failed to expand string 6");
			}
			config->current_entry.what = result.str;
		} else if (STR_EQ(key, "tags")) {
			die("not implemented");
		} else {
			die("Unknown key under entry");
		}

		if(config->current_entry_ready == true) {
			char* thing = config->current_entry.path;
			if (thing == NULL) {
				thing = config->current_entry.source;
			}
			// deploy_symlink(entry);

			config->current_entry_ready = false;
			struct Entry entry = {
				.path = NULL,
				.source = NULL,
				.target = NULL,
				.what = NULL,
				.tags = NULL,
			};
			config->current_entry = entry;
		}
	} else {
		printf("%s\n", section);
		die("Parse error unknown section");
	}

	return 1;
}

struct Config parse_config(char *config_file) {
	bool is_debug = should_debug();

	struct Entry entry = {
		.path = NULL,
		.source = NULL,
		.target = NULL,
		.what = NULL,
		.tags = NULL,
	};
	struct Config config = {
		.current_entry_ready = false,
		.current_entry = entry,
	    .source_root_dir = "",
	    .target_root_dir = "",
		.entries_len = 0,
	    .entries = NULL,
	};

	struct stat st;
	stat(config_file, &st);
	if (!S_ISREG(st.st_mode)) {
		die("Config file is not a regular file");
	}
	if (ini_parse(config_file, parse_config_handler, &config) < 0) {
		die("Can't load config file");
	}

	if (is_debug) {
		fprintf(stderr, "DEBUG: config {\n");
		fprintf(stderr, "  source_root_dir: %s\n", config.source_root_dir);
		fprintf(stderr, "  target_root_dir: %s\n", config.target_root_dir);
		fprintf(stderr, "  entries_len: %d\n", config.entries_len);
		fprintf(stderr, "}\n");
	}

	return config;
}

struct Cli parse_cli(int argc, char *argv[]) {
	if (argc < 2) {
		die("Must pass arguments");
	}

	struct Cli cli = {
	    .command = NULL,
	    .config_file = "",
	    .dry_run = false,
	    .interactive = false,
	    .log = "normal",
	    .help = false,
	};
	enum ParseMode { PARSE_MODE_OPTION, PARSE_MODE_OPTIONVALUE } parse_mode;
	enum ParseCurrent { PARSE_CURRENT_CONFIGDIR, PARSE_CURRENT_LOG } parse_current;
	bool is_debug = should_debug();
	for (int i = 1; i < argc; ++i) {
		if (is_debug) {
			fprintf(stderr, "DEBUG: argv[%d]: %s\n", i, argv[i]);
			fprintf(stderr, "       parse_mode: %d\n", parse_mode);
			fprintf(stderr, "       parse_current: %d\n", parse_current);
		}

		if (parse_mode == PARSE_MODE_OPTION) {
			if (STR_EQ(argv[i], "--config-file")) {
				parse_mode = PARSE_MODE_OPTIONVALUE;
				parse_current = PARSE_CURRENT_CONFIGDIR;
			} else if (STR_EQ(argv[i], "--dry-run")) {
				cli.dry_run = true;
			} else if (STR_EQ(argv[i], "--interactive")) {
				cli.interactive = true;
			} else if (STR_EQ(argv[i], "--log")) {
				parse_mode = PARSE_MODE_OPTIONVALUE;
				parse_current = PARSE_CURRENT_LOG;
			} else if (STR_EQ(argv[i], "--help")) {
				cli.help = true;
			} else {
				if (cli.command == NULL) {
					if (STR_EQ(argv[i], "init") || STR_EQ(argv[i], "list") || STR_EQ(argv[i], "deploy") ||
					    STR_EQ(argv[i], "replace") || STR_EQ(argv[i], "undeploy")) {
						cli.command = argv[i];
					} else {
						die("Subcommand or flag not recognized");
					}
				} else {
					die("Must only pass one command");
				}
			}
		} else if (parse_mode == PARSE_MODE_OPTIONVALUE) {
			if (parse_current == PARSE_CURRENT_CONFIGDIR) {
				cli.config_file = argv[i];
			} else if (parse_current == PARSE_CURRENT_LOG) {
				if (STR_EQ(argv[i], "quiet") || STR_EQ(argv[i], "normal") || STR_EQ(argv[i], "verbose")) {
					cli.log = argv[i];
				} else {
					die("Log value recognized");
				}
			} else {
				die("Invalid value for parse_current");
			}
			parse_mode = PARSE_MODE_OPTION;
		} else {
			die("Invalid parse mode");
		}
	}

	if (cli.command == NULL) {
		die("Must pass a command");
	}

	if (is_debug) {
		fprintf(stderr, "DEBUG: cli {\n");
		fprintf(stderr, " command: %s\n", cli.command);
		fprintf(stderr, " config_file: %s\n", cli.config_file);
		fprintf(stderr, " dry_run: %d\n", cli.dry_run);
		fprintf(stderr, " interactive: %d\n", cli.interactive);
		fprintf(stderr, " log: %s\n", cli.log);
		fprintf(stderr, "} \n");
	}

	return cli;
}
