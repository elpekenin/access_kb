Folder containing things that are referenced from several files. Main reason is that without a package, we would rely on relative paths, which are ugly and prone to errors (eg: moving file around->breakage)

Since typst doesn't support defining packages in a "contained" way, this folder is symlink'ed to `~/.local/share/typst/packages/elpekenin/tfm/0.1.0` which makes `#import "@elpekenin/tfm:0.1.0"` available **system-wide**
