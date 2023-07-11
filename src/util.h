#include <stdbool.h>

#define MAX_TAGS 10
#define MAX_ENTRIES 500
#define STR_EQ(s1, s2) (strcmp(s1, s2) == 0)

struct Entry {
	char const *path;
	char const *source;
	char const *target;
	char const *what;
	char *tags[MAX_TAGS];
};

struct Config {
	bool current_entry_ready;
	struct Entry current_entry;
	char const *source_root_dir;
	char const *target_root_dir;
	unsigned int entries_len;
	struct Entry entries[MAX_ENTRIES];
};

struct Cli {
	char *command;
	char *config_file;
	bool dry_run;
	bool interactive;
	char *log;
	bool help;
};

enum ExpandStringCode {
	ES_SUCCESS,
	ES_FAILURE_DOLLAR_NO_FOLLOWED,
	ES_FAILURE_ESCAPE,
	ES_INVALID_VARIABLE,
};

struct ExpandStringVar {
	char *key;
	char *value;
};

struct ExpandStringVars {
	unsigned int len;
	struct ExpandStringVar vars[100];
};

struct ExpandStringResult {
	enum ExpandStringCode code;
	char *str;
};
int parse_config_handler(void *data, const char *section, const char *key, const char *value);
struct Cli parse_cli(int argc, char *argv[]);
struct Config parse_config(char *config_file);

void die(char *message);
struct ExpandStringResult expand_string(char *input, struct ExpandStringVars vars);
void command_init(struct Cli cli, struct Config config);
void command_list(struct Cli cli, struct Config config);
void command_deploy(struct Cli cli, struct Config config);
void command_replace(struct Cli cli, struct Config config);
void command_undeploy(struct Cli cli, struct Config config);
