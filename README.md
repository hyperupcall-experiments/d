# d

I'm implementing another dotfile manager 😮‍💨

- I stopped working on [dotfox](https://github.com/fox-archives/dotfox) because I [lost confidence](https://forum.nim-lang.org/t/10312#68553) in the [social](https://forum.dlang.org/post/wzoecavcswedkiebcjft@forum.dlang.org) and [technical](https://news.ycombinator.com/threads?id=carterza) leadership for the language I was using, Nim
- [dotmgr](https://github.com/hyperupcall/dotmgr) was supposed to replace it, but it grew too much in scope and it doesn't work on all platforms (Cygwin)

I've learned some things:

- Continue to use symlinks
- Continue to implement undeploy
- Continue to properly test
- Explicitly specify if file/directory
  - That way, a directory can exist at source location (with multiple choices)
- Implement interactive mode
- Implement command to interactively swap out file
- Make status updates one line

## Additional Goals

- Heapless (TODO)
- Multi-platform

## Summary

### Config

Configure which files to manage with an INI-style file.

First, label the location of the source and target directories. All paths are relative to these

```ini
source_root_dir = $pwd/source
target_root_dir = $pwd/target
```

Then, come the variables. By default, `$home` and `$pwd` (relative to the file) are defined, but you can define more (TODO)

```ini
[vars]
cfg = ${XDG_CONFIG_HOME:-$HOME/.config}
data = ${XDG_DATA_HOME:-$HOME/.local/share}
state = ${XDG_DATA_HOME:-$HOME/.local/state}
```

Lastly, list your entries. There are two types:

Tags are TODO

```ini
[entry]
path = .bashrc
what = file
tags = [Bash]

[entry]
source = $cfg/X11/xinitrc
target = $home/.xinitrc
what = file
tags = [X11]
```
