#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/util.h"

int main() {
	struct ExpandStringVars empty_vars = {
		.len = 0,
		.vars = {{0}},
	};

	// No expansions
	{
		struct ExpandStringResult r = expand_string("", empty_vars);
		assert(r.code == ES_SUCCESS);
		assert(STR_EQ("", r.str));
	}
	{
		struct ExpandStringResult r = expand_string("f", empty_vars);
		assert(r.code == ES_SUCCESS);
		assert(STR_EQ("f", r.str));
	}
	{
		struct ExpandStringResult r = expand_string("fox", empty_vars);
		assert(r.code == ES_SUCCESS);
		assert(STR_EQ("fox", r.str));
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
			.len = 1,
			.vars = {
				{.key = "word", .value = "cool"},
			},
		};
		struct ExpandStringResult r = expand_string("${word}/X11", vars);
		assert(r.code == ES_SUCCESS);
		assert(STR_EQ(r.str, "cool/X11"));
	}

	{
		struct ExpandStringVars vars = {
			.len = 1,
			.vars = {
				{.key = "word", .value = "cool"},
			},
		};
		struct ExpandStringResult r = expand_string("fox-${word}", vars);
		assert(r.code == ES_SUCCESS);
		assert(STR_EQ(r.str, "fox-cool"));
	}
	{
		struct ExpandStringVars vars = {
			.len = 2,
			.vars = {
				{.key = "a", .value = "super"},
				{.key = "b", .value = "epic"},
			},
		};
		struct ExpandStringResult r = expand_string("fox-${a}_${b}", vars);
		assert(r.code == ES_SUCCESS);
		assert(STR_EQ(r.str, "fox-super_epic"));
	}
}
