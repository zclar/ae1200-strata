# Linux desktop app

Launch directly from the repository:

```sh
./bin/ae1200-strata
```

The launcher rebuilds `build/libstrata_display.so` whenever the shared C
renderer or its public header changes, then opens the native simulator. Check
the compiler, Python/Tk runtime, and shared library without opening a window:

```sh
./bin/ae1200-strata --check
```

To add **AE1200 Strata** to the current user's Linux application menu:

```sh
./packaging/install-linux.sh
```

This installs only a desktop entry and scalable icon under `XDG_DATA_HOME`
(normally `~/.local/share`). The desktop entry points back to this checkout,
so rerun the installer if the repository moves. It never uses `sudo`.

Remove the menu entry and icon without touching the repository or build:

```sh
./packaging/uninstall-linux.sh
```

Both integration scripts support `--dry-run`.
