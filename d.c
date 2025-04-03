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

struct Entry {
	char const *category;
	char const *source;
	char const *destination;
};
struct Failure {
	char *operation;
	char *reason;
	char *info;
};
void fail(struct Failure failure) {
#define RED "\033[0;31m"
#define BLUE "\033[0;34m"
#define CYAN "\033[0;36m"
#define YELLOW "\033[0;33m"
#define MAGENTA "\033[0;35m"
#define RESET "\033[0m"

	fprintf(stderr,
	        "Application reached a " RED "terminating failure" RESET "...\n" BLUE "Operation: " RESET
	        "Failed to %s\n" CYAN "Reason: " RESET "%s\n",
	        failure.operation, failure.reason);
	if (failure.info != NULL) {
		fprintf(stderr, YELLOW "Extra Information: " RESET "%s\n", failure.info);
	}

	exit(EXIT_FAILURE);
}
void deploy(char *, char *, bool);

int main(int argc, char *argv[]) {
	static bool debug = false;
	if (getenv("DEBUG") != NULL) {
		debug = true;
	}

	char *help_menu = "d: A dotfile manager.\n"
	                  "Commands: <deploy | undeploy | print | compile>";

	enum Command {
		CommandNone,
		CommandDeploy,
		CommandUndeploy,
		CommandPrint,
		CommandCompile,
	} command = CommandNone;

	if (argc < 2) {
		fail((struct Failure){
		    .operation = "validate arguments",
		    .reason = "Must pass a subcommand",
		});
	}
	if (strcmp(argv[1], "deploy") == 0) {
		command = CommandDeploy;
	} else if (strcmp(argv[1], "undeploy") == 0) {
		command = CommandUndeploy;
	} else if (strcmp(argv[1], "print") == 0) {
		command = CommandPrint;
	} else if (strcmp(argv[1], "compile") == 0) {
		command = CommandCompile;
	} else {
		char *strp = NULL;
		if (asprintf(&strp, "\nSubcommand: %s\n---\nHELP MENU:\n%s", argv[1], help_menu) == -1) {
			fail((struct Failure){
			    .operation = "format error with asprintf",
			    .reason = strerror(errno),
			});
		}
		fail((struct Failure){
		    .operation = "validate arguments",
		    .reason = "Subcommand not recognized",
		    .info = strp,
		});
	}

	if (command == CommandCompile) {
		char *cmd = "gcc -Werror=unused-variable -g -fPIC -c " CONFIG_DIR "/dotfiles.c -o " CONFIG_DIR
		            "/dotfiles.o && gcc -shared -o " CONFIG_DIR "/libdotfiles.so " CONFIG_DIR "/dotfiles.o";
		if (system(cmd) == -1) {
			fail((struct Failure){.operation = "system", .reason = strerror(errno)});
		};

		printf("Compiled configuration file\n");

		return EXIT_SUCCESS;
	}

	void *handle = dlopen(CONFIG_DIR "/libdotfiles.so", RTLD_LAZY);
	if (!handle) {
		fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	dlerror();

	struct Entry(**configuration) = (struct Entry **)dlsym(handle, "configuration");
	char *error = dlerror();
	if (error != NULL) {
		fprintf(stderr, "%s\n", error);
		exit(1);
	}

	if (command == CommandPrint)
		printf("[\n");
	for (int i = 0;; i += 1) {
		struct Entry *entrygroup = configuration[i];
		if (entrygroup == NULL) {
			break;
		}

		for (int j = 0;; j += 1) {
			struct Entry entry = entrygroup[j];
			if (entry.source == NULL && entry.destination == NULL && entry.category == NULL) {
				break;
			}

			if (command == CommandPrint) {
				printf("\t{ \"category\": \"%s\", \"source\": \"%s\", \"destination\": \"%s\" }", entry.category,
				       entry.source, entry.destination);
				if (configuration[i + 1] == NULL) {
					printf("\n");
				} else {
					printf(",\n");
				}
				continue;
			}

			char *home = getenv("HOME");
			if (home == NULL) {
				perror("getenv");
				exit(1);
			}
			if (command == CommandDeploy) {
				char source_path[PATH_MAX];
				char destination_path[PATH_MAX];
				snprintf(source_path, PATH_MAX, "%s", entry.source);
				snprintf(destination_path, PATH_MAX, "%s", entry.destination);
				deploy(source_path, destination_path, debug);
			} else if (command == CommandUndeploy) {
				char source_path[PATH_MAX];
				char destination_path[PATH_MAX];
				snprintf(source_path, PATH_MAX, "%s", entry.source);
				snprintf(destination_path, PATH_MAX, "%s", entry.destination);
				if (destination_path[strlen(destination_path) - 1] == '/') {
					destination_path[strlen(destination_path) - 1] = '\0';
				}
				if (unlink(destination_path) == -1) {
					if (errno != ENOENT) {
						fprintf(stderr, "Failed to unlink \"%s\"\n", destination_path);
						perror("unlink");
						exit(1);
					}
				}
			}
		}
	}
	if (command == CommandPrint)
		printf("]\n");
	dlclose(handle);
}

void deploy(char *source_path, char *destination_path, bool debug) {
	// Check trailing slash.
	{
		if (destination_path[strlen(destination_path) - 1] == '/') {
			if (source_path[strlen(source_path) - 1] != '/') {
				fprintf(stderr,
				        "Error: If destination path does have trailing slash, then source path must have it too.\n");
				exit(1);
			}

			destination_path[strlen(destination_path) - 1] = '\0';
			source_path[strlen(source_path) - 1] = '\0';
		} else {
			if (source_path[strlen(source_path) - 1] == '/') {
				fprintf(
				    stderr,
				    "Error: If destination path does not have trailing slash, then source path must not have it either.\n");
				exit(1);
			}
		}

		struct stat st1 = {0};
		if (stat(source_path, &st1) == -1) {
			fprintf(stderr, "Error: Source path  must exist: \"%s\"\n", source_path);
			exit(1);
		}

		struct stat st2 = {0};
		if (stat(destination_path, &st2) != -1) {
			if (S_ISDIR(st1.st_mode) && !S_ISDIR(st2.st_mode)) {
				printf("Error: Destination path is not a directory but the source path is a directory.\n");
				printf("source_path: %s\ndestination_path: %s\n", source_path, destination_path);
				exit(1);
			}
			if (!S_ISDIR(st1.st_mode) && S_ISDIR(st2.st_mode)) {
				printf("Error: Destination path is a directory but the source path is not a directory.\n");
				printf("source_path: %s\ndestination_path: %s\n", source_path, destination_path);
				exit(1);
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
			perror("malloc");
			goto error;
		}

		strcpy(dir, destination_path);
		dirname(dir);
		if (dir == NULL) {
			perror("dirname");
			goto error;
		}
		struct stat st = {0};
		if (stat(dir, &st) == -1) {
			if (errno != ENOENT) {
				perror("stat");
				exit(1);
			}
			printf("Creating directory for: %s\n", dir);
			if (mkdir(dir, 0755) == -1) {
				perror("mkdir");
				exit(1);
			}
		}
		free(dir);

		goto end;
	error:
		free(dir);
		exit(1);
	end:
	}

	struct stat st = {0};
	bool exists = true;
	if (lstat(destination_path, &st) == -1) {
		if (errno != ENOENT) {
			perror("stat");
			exit(1);
		}

		exists = false;
	}

	if (!exists || S_ISLNK(st.st_mode)) {
		if (S_ISLNK(st.st_mode)) {
			if (unlink(destination_path) == -1) {
				perror("unlink");
			}
		}
		if (symlink(source_path, destination_path) == -1) {
			perror("symlink");
			exit(1);
		}
		printf("Symlinked %s to %s\n", source_path, destination_path);
	} else {
		printf("SKIPPING: %s\n", destination_path);
	}
}
