#ifdef __unix__
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <linux/limits.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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
int dircount(char *dir) {
	int file_count = 0;
	DIR *dirp;
	struct dirent *entry;

	dirp = opendir(dir); /* There should be error handling after this */
	while ((entry = readdir(dirp)) != NULL) {
		// if (entry->d_type == DT_REG) { /* If the entry is a regular file */
		file_count++;
		// }
	}
	closedir(dirp);

	return file_count;
}

void _mkdir(const char *dir) {
	char tmp[256];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp), "%s", dir);
	len = strlen(tmp);
	if (tmp[len - 1] == '/')
		tmp[len - 1] = 0;
	for (p = tmp + 1; *p; p++)
		if (*p == '/') {
			*p = 0;
			mkdir(tmp, 0755);
			*p = '/';
		}
	mkdir(tmp, 0755);
}

__attribute__((noreturn)) void die(const char *format, ...) {
	char message[50];

	va_list args;
	va_start(args, format);
	vsprintf(message, format, args);
	va_end(args);

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
	          "  list\n"
	          "    list the currently configured files\n"
	          "  deploy\n"
	          "  undeploy\n"
	          "  replace\n"
	          "\n"
	          "Options:\n"
	          "  --config-file <STRING>\n"
	          "  --dry-run\n"
	          "  --interactive\n"
	          "  --log <normal|verbose|debug>\n"
	          "  --help\n";
	printf("%s", s);
}

/** APPLICATION UTILITY FUNCTIONS */
void deploy_symlink(char *source, char *target, enum EntryWhat what) {
	printf("source: %s\n", source);
	printf("target: %s\n", target);
	printf("deploying\n\n");

	// #ifdef __linux_
	bool source_exists = true;
	struct stat source_st;
	if (stat(source, &source_st) < 0) {
		if (errno != ENOENT) {
			perror("stat1");
			exit(EXIT_FAILURE);
		}

		source_exists = false;
	}

	bool target_exists = true;
	struct stat target_st;
	if (stat(target, &target_st) < 0) {
		if (errno != ENOENT) {
			perror("stat2");
			exit(EXIT_FAILURE);
		}

		target_exists = false;
	}

	if (!target_exists) {
		if (what == WHAT_FILE) {
			char *bn_s = strdup(target);
			char *bn = dirname(bn_s);
			printf("%s\n", bn);

			_mkdir(bn);
			symlink(source, target);
		} else if (what == WHAT_DIR) {

		} else {
			die("Unknown target type 3");
		}
	} else if (S_ISREG(target_st.st_mode)) {
		if (!source_exists) {

		} else if (S_ISREG(source_st.st_mode)) {
			if (what == WHAT_FILE) {

			} else if (what == WHAT_DIR) {

			} else {
				die("Unknown target type 1");
			}
		} else if (S_ISDIR(source_st.st_mode)) {
			die("Directory here is not handled yet 22");
		} else {
			die("Unknown target type 2");
		}
	} else if (S_ISDIR(target_st.st_mode)) {
		die("Directory here is not handled yet");
	} else if (S_ISLNK(target_st.st_mode)) {
		die("Cannot handle symlinks yet");
	} else {
		die("Unknown target type");
	}
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
			while (true) {
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
			while (true) {
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

			char *var = malloc(right - left + 1);
			memset(var, 0, right - left + 1);
			strncat(var, left + 1, right - left - 1);
			bool found = false;
			for (unsigned int i = 0; vars.pairs[i].key != NULL; ++i) {
				if (strcmp(var, vars.pairs[i].key) == 0) {
					strcat(output, vars.pairs[i].value);
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

// TODO --option=value
// TODO: environment variables
struct Options generate_options(int argc, char *argv[]) {
	struct Options options = {
	    .command = COMMAND_UNKNOWN,
	    .config_file = "",
	    .dry_run = false,
	    .interactive = false,
	    .log = LOG_UNKNOWN,
	    .help = false,
	};

	enum ParseMode {
		PARSE_DEFAULT,
		PARSE_CONFIG_DIR,
		PARSE_LOG,
	} parse_mode;

	for (int i = 1; i < argc; ++i) {
		if (should_debug()) {
			fprintf(stderr, "DEBUG: argv[%d]: %s\n", i, argv[i]);
			fprintf(stderr, "       parse_mode: %d\n", parse_mode);
		}

		if (parse_mode == PARSE_DEFAULT) {
			if (strcmp(argv[i], "--config-file") == 0) {
				parse_mode = PARSE_CONFIG_DIR;
			} else if (strcmp(argv[i], "--dry-run") == 0) {
				options.dry_run = true;
			} else if (strcmp(argv[i], "--interactive") == 0) {
				options.interactive = true;
			} else if (strcmp(argv[i], "--log") == 0) {
				parse_mode = PARSE_LOG;
			} else if (strcmp(argv[i], "--help") == 0) {
				options.help = true;
			} else {
				if (options.command == COMMAND_UNKNOWN) {
					if (strcmp(argv[i], "init") == 0) {
						options.command = COMMAND_INIT;
					} else if (strcmp(argv[i], "list") == 0) {
						options.command = COMMAND_LIST;
					} else if (strcmp(argv[i], "deploy") == 0) {
						options.command = COMMAND_DEPLOY;
					} else if (strcmp(argv[i], "undeploy") == 0) {
						options.command = COMMAND_UNDEPLOY;
					} else if (strcmp(argv[i], "replace") == 0) {
						options.command = COMMAND_REPLACE;
					} else {
						die("Failed to recognize subcommand: %s", options.command);
					}
				} else {
					die("Only one command may be passed");
				}
			}
		} else if (parse_mode == PARSE_CONFIG_DIR) {
			options.config_file = argv[i];
			parse_mode = PARSE_DEFAULT;
		} else if (parse_mode == PARSE_LOG) {
			if (strcmp(argv[i], "normal") == 0) {
				options.log = LOG_NORMAL;
			} else if (strcmp(argv[i], "verbose") == 0) {
				options.log = LOG_VERBOSE;
			} else if (strcmp(argv[i], "debug") == 0) {
				options.log = LOG_DEBUG;
			} else {
				die("Failed to recognize log option: %s\n", argv[i]);
			}
			parse_mode = PARSE_DEFAULT;
		} else {
			die("Failed to recognize parse value: %i\n", parse_mode);
		}
	}

	if (options.command == COMMAND_UNKNOWN) {
		show_help();
		die("Must pass a command");
	}

	if (options.config_file == NULL) {
		// TODO
	}

	if (options.log == LOG_UNKNOWN) {
		options.log = LOG_NORMAL;
	}

	if (should_debug()) {
		fprintf(stderr, "DEBUG: cli {\n");
		fprintf(stderr, " command: %i\n", options.command);
		fprintf(stderr, " config_file: %s\n", options.config_file);
		fprintf(stderr, " dry_run: %d\n", options.dry_run);
		fprintf(stderr, " interactive: %d\n", options.interactive);
		fprintf(stderr, " log: %i\n", options.log);
		fprintf(stderr, "} \n");
	}

	if (options.help) {
		show_help();
		exit(0);
	}

	return options;
}

struct Config parse_config(struct Options options) {
	struct Entry entry = {
	    .path = NULL,
	    .source = NULL,
	    .target = NULL,
	    .what = WHAT_UNKNOWN,
	    .tags = NULL,
	};
	struct Config config = {
	    .current_entry_ready = false,
	    .current_entry = entry,
	    .source_root_dir = "",
	    .target_root_dir = "",
	    .entries_len = 0,
	    .entries = NULL,
	    .cli = options,
	};

	struct stat st;
	if (stat(options.config_file, &st) < 0) {
		die("Config file not found: %s", options.config_file);
	}
	if (!S_ISREG(st.st_mode)) {
		die("Config file is not a regular file");
	}
	if (ini_parse(options.config_file, parse_config_handler, &config) < 0) {
		die("Can't load config file");
	}

	if (should_debug()) {
		fprintf(stderr, "DEBUG: config {\n");
		fprintf(stderr, "  source_root_dir: %s\n", config.source_root_dir);
		fprintf(stderr, "  target_root_dir: %s\n", config.target_root_dir);
		fprintf(stderr, "  entries_len: %d\n", config.entries_len);
		fprintf(stderr, "}\n");
	}

	return config;
}

int parse_config_handler(void *data, const char *section, const char *key, const char *value) {
	struct Config *config = (struct Config *)data;

	if (should_debug()) {
		fprintf(stderr, "DEBUG: section: %s\n", section);
		fprintf(stderr, "       key: %s\n", key);
		fprintf(stderr, "       value: %s\n", value);
	}

	char pwddir[PATH_MAX];
	if (getcwd(pwddir, sizeof(pwddir)) == NULL) {
		die("Failed to get cwd");
	}
	strcat(pwddir, "/");
	strcat(pwddir, config->cli.config_file);
	int index = -1;
	for (int i = 0; i < strlen(pwddir) + 1; i++) {
		if (pwddir[i] == '/') {
			index = i;
		}
	}
	pwddir[index] = '\0';

	char *homedir = NULL;
	if ((homedir = getenv("HOME")) == NULL) {
		homedir = getpwuid(getuid())->pw_dir;
	}
	if (homedir == NULL) {
		die("homedir null");
	}

	char cfgdir[300];
	strcat(cfgdir, homedir);
	strcat(cfgdir, "/.config");

	struct ExpandStringVars vars = {
	    .pairs =
	        {
	            {.key = "home", .value = homedir},
	            {.key = "pwd", .value = pwddir},
	            {.key = "cfg", .value = cfgdir},
	            NULL,
	        },
	};

	if (strcmp(section, "") == 0) {
		if (strcmp(key, "source_root_dir") == 0) {
			char *ptr = malloc(strlen(value) + 1);
			strcpy(ptr, value);

			struct ExpandStringResult result = expand_string(ptr, vars);
			if (result.code != ES_SUCCESS) {
				die("Failed to expand string 1");
			}
			config->source_root_dir = result.str;
		} else if (strcmp(key, "target_root_dir") == 0) {
			char *ptr = malloc(strlen(value) + 1);
			strcpy(ptr, value);
			struct ExpandStringResult result = expand_string(ptr, vars);
			if (result.code != ES_SUCCESS) {
				die("Failed to expand string 2");
			}
			config->target_root_dir = result.str;
		} else {
			die("Unknown key under root");
		}
	} else if (strcmp(section, "vars") == 0) {
		fprintf(stderr, "Skipping key: %s\n", key);
		die("not implemented");
	} else if (strcmp(section, "entry") == 0) {
		if (strcmp(key, "path") == 0) {
			if (config->current_entry_ready) {
				die("Forgot to add what");
			}
			config->current_entry_ready = false;

			char *s = malloc(strlen(value) + 1);
			memcpy(s, value, strlen(value) + 1);

			struct ExpandStringResult result = expand_string(s, vars);
			if (result.code != ES_SUCCESS) {
				die("Failed to expand string 3");
			}
			config->current_entry.path = result.str;
		} else if (strcmp(key, "source") == 0) {
			if (config->current_entry_ready) {
				die("Forgot to add what");
			}
			config->current_entry_ready = false;

			char *s = malloc(strlen(value) + 1);
			memcpy(s, value, strlen(value) + 1);
			struct ExpandStringResult result = expand_string(s, vars);

			if (result.code != ES_SUCCESS) {
				die("Failed to expand string 4");
			}
			config->current_entry.source = result.str;
		} else if (strcmp(key, "target") == 0) {
			if (config->current_entry_ready) {
				die("Forgot to add what");
			}
			config->current_entry_ready = false;

			char *s = malloc(strlen(value) + 1);
			memcpy(s, value, strlen(value) + 1);
			struct ExpandStringResult result = expand_string(s, vars);
			if (result.code != ES_SUCCESS) {
				die("Failed to expand string 5");
			}
			config->current_entry.target = result.str;
		} else if (strcmp(key, "what") == 0) {
			config->current_entry_ready = true;

			char *s = malloc(strlen(value) + 1);
			memcpy(s, value, strlen(value) + 1);
			struct ExpandStringResult result = expand_string(s, vars);
			if (result.code != ES_SUCCESS) {
				die("Failed to expand string 6");
			}
			if (strcmp(result.str, "file") == 0) {
				config->current_entry.what = WHAT_FILE;
			} else if (strcmp(result.str, "dir") == 0) {
				config->current_entry.what = WHAT_DIR;
			} else {
				die("Bad what");
			}

			if (config->current_entry.source == NULL) {
				config->current_entry.source = config->current_entry.path;
				config->current_entry.target = config->current_entry.path;
			}

			config->entries_len += 1;
			config->entries[config->entries_len - 1] = config->current_entry;

			config->current_entry.path = NULL;
			config->current_entry.source = NULL;
			config->current_entry.target = NULL;
			config->current_entry.what = WHAT_UNKNOWN;
		} else if (strcmp(key, "tags") == 0) {
			die("not implemented");
		} else {
			die("Unknown key under entry");
		}

		if (config->current_entry_ready == true) {
			char *thing = config->current_entry.path;
			if (thing == NULL) {
				thing = config->current_entry.source;
			}
			// deploy_symlink(entry);

			config->current_entry_ready = false;
			struct Entry entry = {
			    .path = NULL,
			    .source = NULL,
			    .target = NULL,
			    .what = WHAT_UNKNOWN,
			    .tags = NULL,
			};
			config->current_entry = entry;
		}
	} else {
		printf("%s\n", section);
		die("Parse error unknown section");
	}

	return true;
}

void command_init(struct Options cli, struct Config config) {
	printf("%s\n", "init");
}

void command_list(struct Options cli, struct Config config) {
	printf("%s\n", "init");
}

void command_deploy(struct Options cli, struct Config config) {
	for (unsigned int i = 0; i < config.entries_len; ++i) {
		struct Entry entry = config.entries[i];

		char from[200];
		char to[200];
		memset(from, 0, 200);
		memset(to, 0, 200);

		strcat(from, config.source_root_dir);
		strcat(from, "/");
		strcat(from, entry.source);

		strcat(to, config.target_root_dir);
		strcat(to, "/");
		strcat(to, entry.target);
		deploy_symlink(from, to, entry.what);
	}
}

void command_replace(struct Options cli, struct Config config) {
	printf("%s\n", "replace");
}

void command_undeploy(struct Options cli, struct Config config) {
	printf("%s\n", "undeploy");
}
