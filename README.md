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
./bake build "$HOME/.dotfiles/config"
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
#define Done { \
	.category = NULL, .source = NULL, .destination = NULL \
}

static struct Entry bash[] = {
	{
		.category = "bash",
		.source = "/home/edwin/.dotfiles/.bashrc",
		.destination = "/home/edwin/.bashrc"
	},
	{
		.category = "bash",
		.source = "/home/edwin/.dotfiles/.bash_login",
		.destination = "/home/edwin/.bash_login"
	},
	Done
};
static struct Entry zsh[] = {
	{
		.category = "zsh",
		.source = "/home/edwin/.dotfiles/.zshrc",
		.destination = "/home/edwin/.zshrc"
	},
	Done
};

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

The really cool part about this, is that you can use macros. If you don't like
macros, then maybe this software isn't for you. For an example, see my own
[dotfiles.c](https://github.com/hyperupcall/dotfiles/blob/trunk/os-unix/data/dotfiles.c).

Now, you can use `d` like any other dotfile manager:

```console
$ d deploy
$ d undeploy
$ d print
```
