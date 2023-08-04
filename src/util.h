#include <stdbool.h>

#define MAX_TAGS 10
#define MAX_ENTRIES 500

enum EntryWhat {
	WHAT_UNKNOWN,
	WHAT_FILE,
	WHAT_DIR,
};

struct Entry {
	char const *path;
	char const *source;
	char const *target;
	enum EntryWhat what;
	char *tags[MAX_TAGS];
};

enum OPTIONS_COMMAND {
	COMMAND_UNKNOWN,
	COMMAND_INIT,
	COMMAND_LIST,
	COMMAND_DEPLOY,
	COMMAND_UNDEPLOY,
	COMMAND_REPLACE,
};

enum OPTIONS_LOG {
	LOG_UNKNOWN,
	LOG_NORMAL,
	LOG_VERBOSE,
	LOG_DEBUG,
};

struct Options {
	enum OPTIONS_COMMAND command;
	char *config_file;
	bool dry_run;
	bool interactive;
	enum OPTIONS_LOG log;
	bool help;
};

struct Config {
	bool current_entry_ready;
	struct Entry current_entry;
	char const *source_root_dir;
	char const *target_root_dir;
	unsigned int entries_len;
	struct Entry entries[MAX_ENTRIES];
	struct Options cli;
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
	struct ExpandStringVar pairs[100];
};

struct ExpandStringResult {
	enum ExpandStringCode code;
	char *str;
};

void _mkdir(const char *dir);
int dircount(char *dir);
__attribute__((noreturn)) void die(const char *format, ...);
bool should_debug();
void show_help();

void deploy_symlink(char *source, char *target, enum EntryWhat);

struct ExpandStringResult expand_string(char *input, struct ExpandStringVars vars);
struct Options generate_options(int argc, char *argv[]);
struct Config parse_config(struct Options);
int parse_config_handler(void *data, const char *section, const char *key, const char *value);

void command_init(struct Options cli, struct Config config);
void command_list(struct Options cli, struct Config config);
void command_deploy(struct Options cli, struct Config config);
void command_replace(struct Options cli, struct Config config);
void command_undeploy(struct Options cli, struct Config config);
