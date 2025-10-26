#define _GNU_SOURCE

#include <dlfcn.h>
#include <errno.h>
#include <libgen.h>
#include <linux/limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "d.h"

void error(const char *message) {
#define RED "\033[0;31m"
#define RESET "\033[0m"
	fprintf(stderr, RED "%s" RESET "\n", message);
}
void fail(const char *message) {
	error(message);
	exit(EXIT_FAILURE);
}
void deploy(char *, char *, bool, bool);

int main(int argc, char *argv[]) {
	static bool is_debug = false;
	static bool is_dry_run = false;

	char *config_file = NULL;
	char *config_dir = NULL;
	char *config_file2 = NULL;
	char *config_file3 = NULL;
	char *config_filename = NULL;
	char *so_file = NULL;
	char *cmd = NULL;
	char *group_name = NULL;
	void *handle = NULL;

	if (getenv("DEBUG") != NULL) {
		is_debug = true;
	}

	char *help_menu = "d: A dotfile manager.\n"
	                  "Commands: <deploy[ --group=* --dry] | undeploy[ --group=* --dry] | print>\n";

	enum Command {
		CommandNone,
		CommandDeploy,
		CommandUndeploy,
		CommandPrint,
	} command = CommandNone;

	if (argc < 2) {
		fail("Failed to find a subcommand");
	}

	if (strcmp(argv[1], "deploy") == 0) {
		command = CommandDeploy;
	} else if (strcmp(argv[1], "undeploy") == 0) {
		command = CommandUndeploy;
	} else if (strcmp(argv[1], "print") == 0) {
		command = CommandPrint;
	} else {
		char *strp = NULL;
		if (asprintf(&strp, "\nSubcommand: %s\n---\nHELP MENU:\n%s", argv[1], help_menu) == -1) {
			fail("Failed to create help message");
		}
		fail("Failed to recognize subcommand");
	}

	if ((command == CommandDeploy || command == CommandUndeploy) && argc > 2) {
		for (int i = 2; i < argc; i += 1) {
			if (strcmp(argv[i], "--dry") == 0) {
				is_dry_run = true;
			}

			if (strncmp(argv[i], "--group=", 8) == 0) {
				group_name = argv[i] + 8;
			}
		}
	}

	config_file = strdup(CONFIG_FILE);
	if (config_file == NULL) {
		error("Failed to copy config path");
		goto error;
	}
	config_file2 = strdup(config_file);
	if (config_file2 == NULL) {
		error("Failed to copy config path");
		goto error;
	}
	config_file3 = strdup(config_file);
	if (config_file3 == NULL) {
		error("Failed to copy config path");
		goto error;
	}
	config_dir = dirname(config_file2);
	config_filename = basename(config_file3);


	if (asprintf(&so_file, "%s/libdotfiles.so", config_dir) == -1) {
		error("Failed to create library path");
		goto error;
	}


	if (asprintf(&cmd, "gcc -g -fPIC -c %s -o %s/%s.o && gcc -shared -o %s/libdotfiles.so %s/%s.o",
	             config_file, config_dir, config_filename, config_dir, config_dir, config_filename) == -1) {
		error("Failed to create compilation command");
		goto error;
	}

	if (system(cmd) == -1) {
		error("Failed to compile config");
		goto error;
	};

	handle = dlopen(so_file, RTLD_LAZY);
	if (handle == NULL) {
		fprintf(stderr, "%s\n", dlerror());
		goto error;
	}
	dlerror();

	Group **(*getGroups)(void) = (Group **(*)(void))dlsym(handle, "getGroups");
	char *dl_error = dlerror();
	if (dl_error != NULL) {
		fprintf(stderr, "%s\n", dl_error);
		goto error;
	}

	Group *(*getDefaultGroup)(void) = (Group *(*)(void))dlsym(handle, "getDefaultGroup");
	dl_error = dlerror();
	if (dl_error != NULL) {
		fprintf(stderr, "%s\n", dl_error);
		goto error;
	}

	Group **groups = getGroups();
	if (groups == NULL) {
		fprintf(stderr, "Failed to get groups\n");
		goto error;
	}

	Group *current_group = NULL;
	if (group_name == NULL) {
		current_group = getDefaultGroup();
	} else {
		for (int i = 0; groups[i] != NULL; i++) {
			if (strcmp(groups[i]->name, group_name) == 0) {
				current_group = groups[i];
				break;
			}
		}
		if (current_group == NULL) {
			fprintf(stderr, "Group '%s' not found\n", group_name);
			goto error;
		}
	}

	if (current_group->entries == NULL) {
		fprintf(stderr, "Failed to get configuration\n");
		goto error;
	}

	if (command == CommandPrint)
		printf("[\n");
	for (int i = 0;; i += 1) {
		Entry *entrygroup = current_group->entries[i];
		if (entrygroup == NULL) {
			break;
		}

		for (int j = 0;; j += 1) {
			Entry entry = entrygroup[j];
			if (entry.source == NULL && entry.destination == NULL) {
				break;
			}

			if (command == CommandPrint) {
				printf(
				    "\t{ \"source\": \"%s\", \"destination\": \"%s\" }", entry.source,
				    entry.destination
				);
				if (current_group->entries[i + 1] == NULL) {
					printf("\n");
				} else {
					printf(",\n");
				}
				continue;
			}

			char *home = getenv("HOME");
			if (home == NULL) {
				perror("getenv");
				goto error;
			}
			if (command == CommandDeploy) {
				char source_path[PATH_MAX];
				char destination_path[PATH_MAX];
				snprintf(source_path, PATH_MAX, "%s", entry.source);
				snprintf(destination_path, PATH_MAX, "%s", entry.destination);
				deploy(source_path, destination_path, is_debug, is_dry_run);
			} else if (command == CommandUndeploy) {
				char source_path[PATH_MAX];
				char destination_path[PATH_MAX];
				snprintf(source_path, PATH_MAX, "%s", entry.source);
				snprintf(destination_path, PATH_MAX, "%s", entry.destination);
				if (destination_path[strlen(destination_path) - 1] == '/') {
					destination_path[strlen(destination_path) - 1] = '\0';
				}
				if (is_dry_run) {
					printf("[DRY RUN] Would unlink: %s\n", destination_path);
				} else {
					if (unlink(destination_path) == -1) {
						if (errno != ENOENT) {
							fprintf(stderr, "Failed to unlink \"%s\"\n", destination_path);
							perror("unlink");
							goto error;
						}
					}
				}
			}
		}
	}
	if (command == CommandPrint)
		printf("]\n");

error:
	if (handle) dlclose(handle);
	if (so_file) free(so_file);
	if (config_file) free(config_file);
	if (config_file2) free(config_file2);
	if (config_file3) free(config_file3);
	if (cmd) free(cmd);
	exit(1);
}

void deploy(char *source_path, char *destination_path, bool debug, bool dry_run) {
	// Check trailing slash.
	{
		if (destination_path[strlen(destination_path) - 1] == '/') {
			if (source_path[strlen(source_path) - 1] != '/') {
				fprintf(
				    stderr, "Error: If destination path does have trailing slash, then source path must have it too.\n"
				);
				exit(1);
			}

			destination_path[strlen(destination_path) - 1] = '\0';
			source_path[strlen(source_path) - 1] = '\0';
		} else {
			if (source_path[strlen(source_path) - 1] == '/') {
				fprintf(
				    stderr,
				    "Error: If destination path does not have trailing slash, then source path must not have it either.\n"
				);
				exit(1);
			}
		}

		struct stat st1 = {0};
		if (stat(source_path, &st1) == -1) {
			fail("Failed to find source path");
		}

		struct stat st2 = {0};
		if (stat(destination_path, &st2) != -1) {
			if (S_ISDIR(st1.st_mode) && !S_ISDIR(st2.st_mode)) {
				fail("Failed to match directory types");
			}
			if (!S_ISDIR(st1.st_mode) && S_ISDIR(st2.st_mode)) {
				fail("Failed to match directory types");
			}
		}
	}

	if (debug) {
		printf("source_path: %s\ndestination_path: %s\n", source_path, destination_path);
	}

	// Create parent directory if it does not exist.
	{
		char *dir = malloc(strlen(destination_path) + 1);
		if (dir == NULL) {
			error("Failed to allocate path memory");
			goto error;
		}

		strcpy(dir, destination_path);
		dirname(dir);
		if (dir == NULL) {
			error("Failed to extract directory name");
			goto error;
		}
		struct stat st = {0};
		if (stat(dir, &st) == -1) {
			if (errno != ENOENT) {
				fail("Failed to check path status");
			}

			if (dry_run) {
				printf("[DRY RUN] Would create directory: %s\n", dir);
			} else {
				printf("Creating directory for: %s\n", dir);
				if (mkdir(dir, 0755) == -1) {
					perror("mkdir");
					fail("Failed to make directory");
				}
			}
		}
		free(dir);

		goto end;
	error:
		free(dir);
		exit(1);
	end:
		;
	}

	struct stat st = {0};
	bool exists = true;
	if (lstat(destination_path, &st) == -1) {
		if (errno != ENOENT) {
			fail("Failed to check path status");
		}

		exists = false;
	}

	if (!exists || S_ISLNK(st.st_mode)) {
		if (S_ISLNK(st.st_mode)) {
			if (dry_run) {
				printf("[DRY RUN] Would unlink existing symlink: %s\n", destination_path);
			} else {
				if (unlink(destination_path) == -1) {
					error("Failed to remove old link");
				}
			}
		}

		if (dry_run) {
			printf("[DRY RUN] Would symlink %s to %s\n", source_path, destination_path);
		} else {
			if (symlink(source_path, destination_path) == -1) {
				fail("Failed to create link");
			}
			printf("Symlinked %s to %s\n", source_path, destination_path);
		}
	} else {
		printf("SKIPPING: %s\n", destination_path);
	}
}
