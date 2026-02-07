# d

![Sun God](./assets/sun-god.png)

---

A dotfile manager.

## Features

- NOT "SUCKLESS" (IF YOURE'RE TRYING TO CONVINCE ME THAT YOUR SOFTWARE "SUCKS LESS",
  THEN IT ACTUALLY SUCKS!)
- NO "CONFIGURATION FILES" (THE CONCEPT OF "CONFIGURATION FILES" SHOULD NOT
  EXIST!)
- NO "DOCUMENTATION" (WHAT IS THAT?)
- NOT WRITTEN IN RUST (NO, I'M NOT INSANE!)

## Summary

On a more serious note, `d` is your standard dotfile manager, with the twist
that it can be configured using C, hopefully leveraging the ~~cursed~~ amazing C
preprocessor.

### Usage

```bash
git clone git@github.com:hyperupcall-experiments/d
cd ./d
./bake build "$HOME/.dotfiles/config/dotfiles.c"
ln -s "$PWD/d" "$HOME/.local/bin/d"
```

Your `CONFIG_FILE` (see `Bakefile.sh`) should point to a `dotfiles.c` that looks something like:

```c
struct Group {
	char const *name;
	struct Entry **entries;
};
struct Entry {
	char const *category;
	char const *source;
	char const *destination;
};
#define Done { .source = NULL, .destination = NULL }
#define Home CONFIG_HOME

// Each application corresponds to an "entry".
static struct Entry bash[] = {
	{
		.source = Home "/.dotfiles/.bashrc",
		.destination = Home "/.bashrc"
	},
	{
		.source = Home "/.dotfiles/.bash_login",
		.destination = Home "/.bash_login"
	},
	Done
};
static struct Entry zsh[] = {
	{
		.source = Home "/.dotfiles/.zshrc",
		.destination = Home "/.zshrc"
	},
	Done
};

// Group entries that should be deployed together.
static struct Group group1 = {
	.name = "Linux desktop",
	.entries = (struct Entry *[]){
		bash,
		zsh
		NULL
	}
};
static struct Group group2 = {
	.name = "Linux laptop",
	.entries = (struct Entry *[]){
		zsh,
		NULL
	}
};

// Finally, list all groups and configure a default group.
static struct Group** groups = (struct Group *[]){
	&group1,
	&group2,
	NULL
};
struct Group **getGroups() {
	return groups;
}
struct Group *getDefaultGroup() {
	return &group2;
}
```

In summary, each dotfile entry corresponds to some application and can have multiple dotfile files or directories. Then, group them by how you would like to deploy them. You must write `getGroups()` and `getDefaultGroup()` so `d` can see and use the groups that you have.

The really cool part about this, is that you can use macros! This is your chance to be creative! See
[my dotfiles.c](https://github.com/hyperupcall/dotfiles/blob/trunk/os-unix/data/dotfiles.c) for inspiration.

Now, you can use `d` like any other dotfile manager:

```console
$ d deploy
$ d undeploy
$ d print
```
