#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/util.h"

int main() {
	struct ExpandStringVars empty_vars = {
		.pairs = {NULL},
	};

	// No expansions
	{
		struct ExpandStringResult r = expand_string("", empty_vars);
		assert(r.code == ES_SUCCESS);
		assert(strcmp("", r.str) == 0);
	}
	{
		struct ExpandStringResult r = expand_string("f", empty_vars);
		assert(r.code == ES_SUCCESS);
		assert(strcmp("f", r.str) == 0);
	}
	{
		struct ExpandStringResult r = expand_string("fox", empty_vars);
		assert(r.code == ES_SUCCESS);
		assert(strcmp("fox", r.str) == 0);
	}

	// Bad syntax: Dollar
	{
		struct ExpandStringResult r = expand_string("$", empty_vars);
		assert(r.code == ES_FAILURE_DOLLAR_NO_FOLLOWED);
	}
	{
		struct ExpandStringResult r = expand_string("fox$", empty_vars);
		assert(r.code == ES_FAILURE_DOLLAR_NO_FOLLOWED);
	}
	{
		struct ExpandStringResult r = expand_string("$ ", empty_vars);
		assert(r.code == ES_FAILURE_DOLLAR_NO_FOLLOWED);
	}
	{
		struct ExpandStringResult r = expand_string("fox$ ", empty_vars);
		assert(r.code == ES_FAILURE_DOLLAR_NO_FOLLOWED);
	}
	{
		struct ExpandStringResult r = expand_string("fox${", empty_vars);
		assert(r.code == ES_FAILURE_DOLLAR_NO_FOLLOWED);
	}
	{
		struct ExpandStringResult r = expand_string("fox$}", empty_vars);
		assert(r.code == ES_FAILURE_DOLLAR_NO_FOLLOWED);
	}
	{
		struct ExpandStringResult r = expand_string("\\$$", empty_vars);
		assert(r.code == ES_FAILURE_DOLLAR_NO_FOLLOWED);
	}
	{
		struct ExpandStringResult r = expand_string("\\$$ ", empty_vars);
		assert(r.code == ES_FAILURE_DOLLAR_NO_FOLLOWED);
	}
	{
		struct ExpandStringResult r = expand_string("$\\{{a}", empty_vars);
		assert(r.code == ES_FAILURE_DOLLAR_NO_FOLLOWED);
	}

	// Bad syntax: Variable
	{
		struct ExpandStringResult r = expand_string("${}", empty_vars);
		assert(r.code == ES_INVALID_VARIABLE);
	}
	{
		struct ExpandStringResult r = expand_string("${2}", empty_vars);
		assert(r.code == ES_INVALID_VARIABLE);
	}
	{
		struct ExpandStringResult r = expand_string("${fox2}", empty_vars);
		assert(r.code == ES_INVALID_VARIABLE);
	}
	{
		struct ExpandStringResult r = expand_string("${2fox}", empty_vars);
		assert(r.code == ES_INVALID_VARIABLE);
	}

	// Bad syntax: Escaping
	{
		struct ExpandStringResult r = expand_string("f{ox${word}", empty_vars);
		assert(r.code == ES_FAILURE_ESCAPE);
	}
	{
		struct ExpandStringResult r = expand_string("f}ox${word}", empty_vars);
		assert(r.code == ES_FAILURE_ESCAPE);
	}

	// Working
	{
		struct ExpandStringVars vars = {
			.pairs = {
				{.key = "word", .value = "cool"},
				NULL,
			},
		};
		struct ExpandStringResult r = expand_string("${word}/X11", vars);
		assert(r.code == ES_SUCCESS);
		assert(strcmp(r.str, "cool/X11") == 0);
	}

	{
		struct ExpandStringVars vars = {
			.pairs = {
				{.key = "word", .value = "cool"},
				NULL,
			},
		};
		struct ExpandStringResult r = expand_string("fox-${word}", vars);
		assert(r.code == ES_SUCCESS);
		assert(strcmp(r.str, "fox-cool") == 0);
	}
	{
		struct ExpandStringVars vars = {
			.pairs = {
				{.key = "a", .value = "super"},
				{.key = "b", .value = "epic"},
				NULL,
			},
		};
		struct ExpandStringResult r = expand_string("fox-${a}_${b}", vars);
		assert(r.code == ES_SUCCESS);
		assert(strcmp(r.str, "fox-super_epic") == 0);
	}
}
