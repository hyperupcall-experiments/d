# d

![Sun God](./assets/sun-god.png)

---

A dotfile manager.

## Features

- NOT "SUCKLESS" (IF YOU HAVE TO CONVINCE ME THAT YOUR SOFTWARE "SUCKS LESS",
  THEN IT ACTUALLY SUCKS!)
- NO "CONFIGURATION FILES" (THE CONCEPT OF "CONFIGURATION FILES" SHOULD NOT
  EXIST!)
- NO "DOCUMENTATION" (WHAT IS THAT?)
- NOT WRITTEN IN RUST (NO, I'M NOT INSANE!)

## Summary

On a more serious note, `d` is your standard dotfile manager, with the twist
that it can be configured using C, hopefully leveraging the ~~cursed~~ C
preprocessor. It's meant to be small and only does what it says it does.

It approaches reconciliation using symlinks. It doesn't support templates or any
of that nonsense.

A disclaimer for those curious to use `d`: currently, the code is dogshit. But
I'm sure that many of you wouldn't mind one bit.

### Usage

```bash
git clone git@github.com:fox-incubating/d
cd ./d
gcc -DCONFIG_DIR=\"$HOME/.dotfiles/config\" ./d.c -o ./d
ln -s "$PWD/d" "$HOME/.local/bin/d"
```

Your `CONFIG_FILE` should have a file `dotfiles.c` that looks something like:

```c
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
		.category = "bash"
		.source = "/home/edwin/.dotfiles/.bashrc",
		.destination = "/home/edwin/.bashrc"
	},
	Done
};

struct Entry *configuration[] = {
	bash,
	NULL
};
```

Note the `configuration` object; `d` will manage all entries contained within it.

The really cool part about this, is that you can use macros. If you don't like
macros, then maybe this software isn't for you. For an example, see my own
[dotfiles.c](https://github.com/hyperupcall/dotfiles/blob/trunk/os-unix/data/dotfiles.c).
Later I'll probably support some sort of `get_configuration()` to allow the use
of runtime shenanigans.

Whenever you edit, you must run `d compile` to compile the C program into a
shared object file. It'll be dynamically loaded on subsequent invocations. Maybe
I'll add a file watcher sometime.

Now, you can use `d` like any other dotfile manager:

```console
$ d deploy
$ d undeploy
$ d print
```
