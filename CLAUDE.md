# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

PHP-GTK3: a PHP extension written in C++ that binds GTK+ 3 (plus GDK, Pango, GtkSourceView, and optionally Glade, libwnck, WebKit) to PHP, built on [PHP-CPP](https://github.com/CopernicaMarketingSoftware/PHP-CPP). Target platforms are **Linux and Windows** (macOS code exists but is not maintained).

Reference semantics against **GTK 3.24.x** (specifically 3.24.48) — never GTK 4. PHP-facing behavior must stay compatible with **PHP 7.4 and newer**.

## Build

Linux / MSYS2-MinGW build is the `Makefile`:

```bash
make -j12                                    # build gtk3.so
make clean
make install                                 # copies .so to EXTENSION_DIR, .ini to INI_DIR
make objects                                 # compile only, no link
make compile_commands                        # bear -- make objects  → compile_commands.json for clangd/clang-tidy
```

Key variables (override on the command line):

- `PHP_CONFIG=/opt/php/php-8.4.14/bin/php-config` — selects the target PHP; falls back to `php-config` on PATH.
- `PHPCPP_STATIC=/path/libphpcpp.a.2.4.13` — link a specific static PHP-CPP by full path instead of `-lphpcpp`; needed because a system-installed libphpcpp is often built for a different PHP version. The version is parsed out of the filename and reported via `PHPGTK_PHPCPP`.
- Feature switches: `WITH_WEBKIT=1`, `WITH_GLADEUI=1`, `WITH_LIBWNCK=1`, `WITH_MAC_INTEGRATION=1`, `WEBVIEW2_SDK=C:/WebView2SDK` (Windows WebView2 path). These are compared with `ifeq ($(X),1)` — passing `WITH_WEBKIT=0` really disables the feature.

`./buildall.sh` builds/installs for each PHP version listed in its `BUILDS` array (needs root; re-execs itself with sudo).

Makefile gotchas worth knowing before editing it:

- Object files have **no source prerequisites** — they are only rebuilt when missing. `version.o` is force-rebuilt via a `FORCE` prerequisite so the git hash / build date stay current. After editing a `.cpp` you must delete its `.o` (or `make clean`) to get a rebuild.
- `COMPILER_FLAGS` must keep ending in `-o`; the object rule appends `$@ source.cpp` right after it. Never append flags below that line.

Windows native (MSVC) builds use a hand-made Visual Studio project against MSYS2 GTK headers — see `docs/compile-windows.md`. `docs/compile-linux.md`, `compile-docker.md`, `appimage.md`, `webkit.md`, `webview2-windows.md` cover the other paths.

## Lint

```bash
./lint.sh                 # clang-tidy + clang-format --dry-run over src/**, main.cpp, php-gtk.cpp
./lint.sh --fix           # apply clang-tidy --fix and clang-format -i
./lint.sh --no-tidy       # format only
./lint.sh --no-format     # tidy only
./lint.sh --fail-fast
CLANG_TIDY=clang-tidy-18 CLANG_FORMAT=clang-format-18 JOBS=8 ./lint.sh
```

Checks come from `.clang-tidy` (bugprone/performance/readability/modernize/cppcoreguidelines with a curated deny-list) and `.clang-format` (Google, 2-space, 100 cols, `SortIncludes: false`).

**There is no test suite.** Verify changes by building and running a PHP script — `examples/*.php` are the runnable samples.

CI (`.github/workflows/codechecks.yml`) is static analysis only: it never links the `.so`. It generates a compile database with all `WITH_*` features on, then runs cpp-linter restricted to changed files *and changed lines* — the full-codebase backlog (~9.7k findings) is deliberately not gated on.

## Architecture

**Three layers:**

1. `main.cpp` (~300 KB, one function) — `get_module()` registers every PHP class, method, constant, and enum with PHP-CPP. `main.h` is just the include manifest, with `#ifdef WITH_GLADEUI/WITH_WEBKIT/WITH_LIBWNCK/WITH_MAC_INTEGRATION` guards mirroring the Makefile.
2. `php-gtk.cpp` / `php-gtk.h` — shared runtime helpers: parameter checking, type-name conversion, the generic signal-callback trampoline, `cobject_to_phpobject()`, and the callback-exception reporting API.
3. `src/<Namespace>/` — one C++ class per wrapped C type: `src/G/`, `src/Gdk/`, `src/Gtk/` (~229 files), `src/Glade/`, `src/GtkSourceView/`, `src/Pango/`, `src/WebKit/`, `src/libwnck/`.

**Wrapper conventions** (follow the nearest sibling class rather than inventing):

- Class name is the GTK type with a trailing underscore: `GtkButton_` in `src/Gtk/GtkButton.{h,cpp}`, registered to PHP as `"GtkButton"`.
- Everything derives from `GObject_ : Php::Base`, which owns the single `gpointer *instance` plus `get_instance()`/`set_instance()`. The C++ inheritance chain mirrors GTK's (`GtkButton_ → GtkBin_ → GtkContainer_ → GtkWidget_ → GObject_`), and `main.cpp` mirrors it again with `.extends(...)`.
- `__construct()` assigns `instance = (gpointer *)gtk_x_new()`. Static factory methods (`new_with_label`, …) construct a wrapper, `set_instance()` the result, and return `Php::Object("GtkButton", ptr)`.
- Returning an arbitrary GObject to PHP: `cobject_to_phpobject()` picks the PHP class name from `g_type_name(G_TYPE_FROM_INSTANCE(...))` at runtime, so the PHP class registered in `main.cpp` must be named exactly after the GType.
- Enums/flags are registered as `Php::Class<Php::Base>` holding only `constant()` entries (e.g. `GtkButtonsType::OK`), not as wrappers.
- Deprecated GTK APIs are kept as stubs that emit `Php::deprecated << ...` rather than being removed.
- Unimplemented methods `throw Php::Exception("Class_::method not implemented")`.

**Signals and callbacks.** `GObject_::connect()`/`connect_after()` build a heap `st_callback` from the PHP callable, query the signal via `g_signal_query()` (zero-initialize the `GSignalQuery` — detailed signals like `notify::visible` fail `g_signal_lookup()` but still connect), and route emission through a generic trampoline that converts GValues to PHP values.

**Exceptions must never unwind through GLib C frames.** Every callback catches PHP `Throwable`s at the boundary and calls `phpgtk_report_callback_exception(message, code, context)`, where `context` is a string literal naming the installing site (signal name, or e.g. `"Gtk::timeout_add"`). The handler is *registered* (`phpgtk_set_exception_handler`, exposed as `Gtk::set_exception_handler()`), not looked up by name — a pending Zend exception silently no-ops every later `Php::call()`. Default behavior is a `g_critical()` and the app keeps running. When adding a new callback entry point, wire it into this path.

**Build metadata.** `version.cpp` is the one force-rebuilt translation unit; it turns the `-D` flags from the Makefile into `PHPGTK_BUILD_INFO`, `PHPGTK_FEATURES`, `PHPGTK_PHPCPP` constants (also exposed as read-only ini directives so they show in `phpinfo()`). Feature strings are assembled from the same `#ifdef`s the Makefile sets, so they cannot drift from what was compiled.

**`gen/`** is a one-off PHP scaffolding generator (`gen/run.php` reads `gen/defs.txt`) used to bootstrap wrapper skeletons. Generated code is hand-edited afterwards; don't regenerate over existing wrappers.

## Working on bindings

Look up the real GTK 3.24 C signature first (ownership, nullability, constness, deprecation status), then mirror it. Mind GTK reference counting (`g_object_ref`/`unref`) so PHP object lifetimes don't free GTK objects early or leak them. Keep observable PHP behavior identical across PHP 7.4 and 8.x.

Adding a method requires touching three places: the header declaration, the `.cpp` implementation, and the `.method<&Class_::name>("name")` registration in `main.cpp`. A new class additionally needs an include in `main.h`.
