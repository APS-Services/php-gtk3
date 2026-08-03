# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

php-gtk3 is a PHP extension written in C++ that binds GTK 3 (plus GDK, Pango, GtkSourceView, and
optionally Glade, libwnck, WebKit2GTK/WebView2) to PHP, using
[PHP-CPP](https://github.com/CopernicaMarketingSoftware/PHP-CPP) as the Zend-API abstraction layer.

The module is named **`gtk3`** everywhere: `NAME = gtk3` in the Makefile, `Php::Extension("gtk3")`
in `main.cpp`, `extension=gtk3` in `gtk3.ini`. PHP derives the library name from it — `gtk3.so` on
Linux, `php_gtk3.dll` on Windows. Keep those four in sync; a mismatch means `php -m` /
`extension_loaded('gtk3')` disagree with the ini directive.

Hard constraints on all changes:

- **GTK 3.24.x** is the reference API (~3.24.48). Never use GTK 4 APIs.
- **PHP 7.4 through current 8.x** must all keep working; the same sources are compiled once per PHP
  version.
- The Makefile compiles with `-std=c++11` (lint.sh runs clang-tidy with `-std=c++17`, and the
  Windows MSVC project uses `/std:c++17`, so a clang-tidy-clean or MSVC-clean file can still fail
  the Linux build — the C++11 setting wins).
- Linux and Windows are the supported targets; macOS bits (`WITH_MAC_INTEGRATION`) are legacy.

## Build (Linux / MSYS2-MinGW)

Building requires a matching PHP-CPP static/shared lib per PHP version. Building the extension is
_not_ required for most source-level tasks (see the CI section — CI itself never links).

```sh
# Single version (defaults already target the distro PHP 8.4)
make PHP_CONFIG=/usr/bin/php-config8.4 INI_DIR=/etc/php/8.4/mods-available/ -j12

# All enabled versions, install .so into each PHP's extension dir (re-execs itself with sudo)
./buildall.sh                        # PHPCPP_BASE=/mnt/share/dev/code/PHP-CPP/dist by default

make objects                         # compile only, no link (what CI uses)
make install                         # copies gtk3.so to EXTENSION_DIR, gtk3.ini to INI_DIR
make clean
make compile_commands                # bear -- make objects, for clangd/clang-tidy
```

Makefile facts worth knowing before editing it or debugging a stale build:

- **Object files have no source prerequisites** — the `${OBJECTS}:` rule rebuilds them only when the
  `.o` is missing. After editing a `.cpp` you must delete its `.o` (or `make clean`) to get a
  rebuild. `make clean` is also required between PHP versions: `.o` files are not version-tagged.
- `version.o` is the one exception, force-rebuilt via a `FORCE` prerequisite so the git hash and
  build date stay current on incremental builds.
- `COMPILER_FLAGS` must keep ending in `-o`; the object rule appends `$@ source.cpp` right after
  it. Never append flags below that line.
- Feature switches are compared with `ifeq ($(X),1)`, not `ifdef` — so `WITH_WEBKIT=0` really does
  disable the feature (buildall.sh relies on this). Flags: `WITH_WEBKIT`, `WITH_GLADEUI`,
  `WITH_LIBWNCK`, `WITH_MAC_INTEGRATION`, plus `WEBVIEW2_SDK` on Windows. Documented in the
  Makefile header comment.
- `PHPCPP_STATIC=/path/libphpcpp.a.x.y.z` links a specific PHP-CPP by full path — use it to avoid
  picking up a system libphpcpp built against a different PHP version. The version is parsed out of
  the filename and reported as `PHPGTK_PHPCPP`.
- `pkg-config` names are pinned: `gtk+-3.0`, `gtksourceview-3.0`, `gladeui-2.0`, `libwnck-3.0`,
  `webkit2gtk-4.1`.
- `PHP_CONFIG`/`INI_DIR` default to the distro PHP 8.4 (`/usr/bin/php-config8.4`,
  `/etc/php/8.4/mods-available/`), with a fall back to `php-config` from `$PATH`. `build-*.sh` is
  gitignored (personal build wrappers); `buildall.sh` is tracked.

**The `php-config` used here must be the same one PHP-CPP was built against.** `buildall.sh` and
`PHP-CPP/build-dist.sh` both use the distro `php-config<version>` for exactly this reason. Nothing
checks the pairing: compiling against one PHP's headers and linking a libphpcpp built against
another's produces a `.so` that loads, runs, and then corrupts the heap during
`php_module_shutdown()` — the process aborts on exit with `free(): invalid pointer` after the script
has already finished successfully. When a build behaves that way, suspect the toolchain pairing
before the source. `PHPGTK_PHPCPP` / `ini_get('gtk3.phpcpp')` reports which libphpcpp a given binary
actually linked.

### Build metadata

`version.cpp` is a deliberately tiny translation unit that turns the Makefile's `-D` flags into the
`PHPGTK_BUILD_INFO`, `PHPGTK_FEATURES` and `PHPGTK_PHPCPP` constants (also registered as
`Php::Ini::Place::System` directives so they appear in `phpinfo()`). The feature string is assembled
from the same `#ifdef`s the Makefile sets, so it cannot drift from what was actually compiled.

## Windows

Windows is a first-class target, but it is built differently from Linux.

**Two Windows paths exist:**

1. **Native MSVC** (`docs/compile-windows.md`, the current one) — a hand-made Visual Studio "Blank
   C++ project" that compiles PHP-CPP's and php-gtk3's `.cpp` files together into
   `Release/php_gtk3.dll`. There is no `.vcxproj`/`.sln` in the repo; the doc is the project
   definition. It builds against PHP built from source with the php-sdk-binary-tools, and against
   **GTK headers/import libs from MSYS2 mingw64** (`pacman -S mingw-w64-x86_64-gtk3`, `-gtksourceview3`,
   `-glade`, `-libglade`, `-gdk-pixbuf2`, `-glib2`).
2. **MSYS2/MinGW** — the same `Makefile` works there; `ifeq ($(OS),Windows_NT)` inside the
   `WITH_WEBKIT` block switches the web backend. `docs/compile-windows-old.md` is the superseded
   MinGW-era writeup, kept for reference.

**What must be excluded from the Windows source list** (mirrors the Makefile's `filter-out`):

- `src/Gtk/GtkosxApplication.cpp` — macOS only.
- `src/libwnck/*.cpp` — X11 only. (These are also `#ifdef WITH_LIBWNCK`-guarded internally, so they
  compile to nothing without the flag, but the MSVC doc removes them outright.)
- `src/WebKit/WebKitWebView_Unix.cpp` and `src/WebKit/WebKitWebView_Windows.cpp` — **never compile
  these directly.** `WebKitWebView.cpp` `#include`s one of them based on `_WIN32`; adding them to
  the build produces duplicate symbols.

**MSVC settings that matter** (full list in `docs/compile-windows.md`): dynamic library (.dll), x64
Release, `/std:c++17`, `/MT`, `/W0`, `/sdl-`, and a long preprocessor list including `PHP_WIN32`,
`ZEND_WIN32`, `BUILDING_PHPCPP`, `_NOEXCEPT=noexcept`, `GTK_COMPILATION`/`GDK_COMPILATION`/
`GLIB_COMPILATION`/`GIO_COMPILATION`/`GDK_PIXBUF_COMPILATION`, and a `PHP_VERSION_ID` matching the
target PHP. PHP-CPP itself needs manual patching for Windows (unmerged upstream PRs, plus renaming
`zend/string.h` to `strings.h` and fixing its includes) — that is a known, documented wart.

**Deployment on Windows** is a copy job, not an installer: `php_gtk3.dll` into `ext\`,
`extension=gtk3` in `php.ini`, ~40 MSYS2 `mingw64/bin/*.dll` runtime libraries next to `php.exe`,
`mingw64/share` themes/icons, and a `gdk-pixbuf-query-loaders.exe --update-cache` run plus
`lib/gdk-pixbuf-2.0`. See the "Running and packing" section of `docs/compile-windows.md`.

**WebView on Windows is a different engine.** `WITH_WEBKIT=1` means WebKit2GTK on Linux and
**Microsoft Edge WebView2** on Windows, selected by `_WIN32` at compile time — same PHP-facing
`GtkWebView` API either way. WebView2 needs the SDK at build time (`WEBVIEW2_SDK`, default
`C:/WebView2SDK`, linking `WebView2LoaderStatic.lib`) but ships no extra DLLs, since the runtime
comes with Edge on Windows 10/11. Details in `docs/webview2-windows.md` and `docs/webkit.md`.

When touching anything under `src/WebKit/`, check both `_WIN32` branches — the two implementation
files are independent and easily drift apart.

## Lint

```sh
./lint.sh                    # clang-tidy + clang-format --dry-run over src/**, main.cpp, php-gtk.cpp
./lint.sh --fix              # apply clang-tidy --fix and clang-format -i
./lint.sh --no-tidy          # format only (much faster)
./lint.sh --no-format        # tidy only
./lint.sh --fail-fast
CLANG_TIDY=clang-tidy-17 CLANG_FORMAT=clang-format-17 JOBS=8 ./lint.sh
```

Checks live in `.clang-tidy` (bugprone/performance/readability/modernize/cppcoreguidelines with a
curated deny-list) and `.clang-format` (Google, 2-space, 100 cols). Note `SortIncludes: false` — do
not reorder includes.

## Tests

There is no test suite. Verification is manual and done by the maintainer: build, install the `.so`,
and run a script against it — `examples/*.php` are the working reference scripts
(`php8.4 examples/exception_handler.php`). GUI code needs a display; `Xvfb` is the usual headless
workaround, and `GDK_BACKEND=x11` is needed for `GtkStatusIcon` under Wayland.

`examples/callback_lifetime.php` is the closest thing to a regression test — 23 self-checking
assertions over every callback entry point, exiting non-zero on failure:

```sh
# fresh build, without installing it (-n is required, see below)
xvfb-run -a php8.4 -n -dextension=./gtk3.so examples/callback_lifetime.php

# or the installed build
xvfb-run -a php8.4 examples/callback_lifetime.php
```

**`-n` is not optional when combining `-dextension=` with an installed copy.** Without it PHP reads
the ini files too, loads the extension a second time, and segfaults — which looks exactly like a bug
in the build under test. `-n` also drops every other extension, which this script does not need.

It deliberately drives callback *teardown* (widget destroy, source removal, sort-function
replacement), not just registration, because that is the half where the destroy notifies run. Run it
after touching anything in `GObject_::connect`, `Gtk::timeout_add`, or the `set_*_func` family.

For source-level changes, `./lint.sh` plus reading the code is the practical check — say what you
did and did not verify rather than assuming a build confirmed it.

## CI

`.github/workflows/codechecks.yml` runs static analysis only — it generates `compile_commands.json`
via `make compile_commands` with all `WITH_*` features on, against PHP-CPP _headers only_ (no
PHP-CPP build, no link step), then runs cpp-linter restricted to changed files _and_ changed lines.
It is **report-only**: there is a large pre-existing findings backlog (~9.7k as of 2026-07), so lint
findings do not fail the build. The workflow's `GTKFLAGS` package list must mirror the Makefile's.

## Architecture

### Two layers

1. **Wrapper classes** (`src/<Namespace>/<GtkType>.{h,cpp}`) — one C++ class per GTK/GDK/Pango type,
   named `<GtkType>_` (trailing underscore), e.g. `GtkButton_` in `src/Gtk/GtkButton.cpp`. Methods
   mirror the GTK C function names with the type prefix stripped (`gtk_button_set_label` →
   `GtkButton_::set_label`). Namespaces: `src/G/`, `src/Gdk/`, `src/Gtk/` (~229 files),
   `src/Glade/`, `src/GtkSourceView/`, `src/Pango/`, `src/WebKit/`, `src/libwnck/`.
2. **Registration** (`main.cpp`, ~295k, a single `get_module()`) — every PHP-visible class,
   constant, and inheritance edge is declared here. `main.h` is just the include list for it, with
   `#ifdef WITH_*` guards mirroring the Makefile.

`php-gtk.cpp` / `php-gtk.h` sit alongside as the shared runtime: parameter checking, type-name
conversion, the generic callback trampoline, `cobject_to_phpobject()`, and the callback-exception
reporting API.

### The instance pointer and the class hierarchy

`GObject_` (`src/G/GObject.h`) is the root: it extends `Php::Base` and holds
`gpointer *instance`, the underlying C object. Every wrapper inherits through the same chain GTK
itself uses — `GObject_` → `GtkWidget_` → `GtkContainer_` → `GtkBin_` → `GtkButton_` → … — so the
C++ hierarchy mirrors `docs/reference-objects.md`.

Two idioms recur everywhere:

- Constructors set `instance = (gpointer *)gtk_foo_new();` in `__construct()`.
- Static factory methods (`new_with_label`, etc.) allocate a wrapper, `set_instance(...)`, and
  return `Php::Object("GtkButton", return_parsed)`.

`cobject_to_phpobject()` (`php-gtk.cpp`) is the generic reverse direction: wrap an arbitrary
`gpointer` into a PHP object whose class name comes from `g_type_name(G_TYPE_FROM_INSTANCE(...))` —
so a PHP class registered in `main.cpp` must be named exactly after its GType.

### Adding or extending a binding

Look up the real GTK 3.24 C signature first (ownership, nullability, constness, deprecation status)
and mirror it, minding `g_object_ref`/`unref` so PHP object lifetimes neither free GTK objects early
nor leak them.

Adding a method to an existing class: implement in `src/.../X.cpp`, declare in `X.h`, then add one
`x.method<&X_::name>("name");` line in `main.cpp`.

Adding a whole class requires four edits:

1. `src/<Ns>/<GtkType>.{h,cpp}` — inherit from the wrapper of the GTK parent type.
2. `#include` it in `main.h` (respecting the existing feature `#ifdef` blocks).
3. In `main.cpp`: `Php::Class<GtkFoo_> gtkfoo("GtkFoo"); gtkfoo.extends(gtkparent);` then the
   `method<>` lines and any `constant()` lines. **The parent's `Php::Class` variable must be
   declared earlier in `get_module()`** — declaration order in that function is a real dependency.
4. `extension.add(std::move(gtkfoo));` — classes are moved into the extension in batches near the
   end of each section; a class that is never `add`ed silently does not exist in PHP.

Pure-enum classes are registered as `Php::Class<Php::Base>` holding only `constant()` entries
(e.g. `GtkStateType`, `GdkWindowType`).

### Conventions inside wrappers

Follow the nearest sibling class rather than inventing a new shape.

- **Deprecated GTK APIs**: keep the method registered, comment out the body, and emit
  `Php::deprecated << "x is deprecated on Gtk 3.10" << '\n';`.
- **Unimplemented**: `throw Php::Exception("GtkButton_::get_event_window not implemented");`.
- **Parameter validation**: `phpgtk_check_parameter(parameters, n, Php::Type::X, required, "GtkY")`
  (1-based `param`; throws when required, warns and returns false when optional).
- **GValue conversion**: `phpgtk_get_gvalue()` / `phpgtk_get_phpvalue()` (declared in `main.h`).

### Signals and the exception boundary

`GObject_::connect()` builds an `st_callback` holding the PHP callable plus the signal's
`n_params`/`param_types` from `g_signal_query()` (zero-initialize the `GSignalQuery` — detailed
signals like `notify::visible` fail `g_signal_lookup()` but still connect), then attaches
`g_cclosure_new_swap(G_CALLBACK(connect_callback), ...)`. `connect_callback()` walks the varargs
switching on `G_TYPE_FUNDAMENTAL(param_types[i])` to marshal each argument into a `Php::Value`.
Several fundamental types (`G_TYPE_POINTER`, `G_TYPE_BOXED`, `G_TYPE_PARAM`, …) are deliberately
skipped — they are not yet marshalled. `generic_callback()` in `php-gtk.cpp` is a second, similar
marshaller used where the userdata position is unknown.

**A PHP throwable must never unwind across GLib's C frames** (`gtk_main`, `gtk_dialog_run`) — that
is UB and can `std::terminate`. So callbacks catch `Php::Throwable` (not just `Php::Exception`;
PHP `Error`s arrive as `Php::Error`), leave the catch block, and then call
`phpgtk_report_callback_exception(message, code, context)`, where `context` names the installing
site (the signal name, or e.g. `"Gtk::timeout_add"`). Consequence: a PHP `try/catch` around
`Gtk::main()` never sees handler exceptions — applications observe them via
`Gtk::set_exception_handler(?callable)` (message, signal name, code), or they land on stderr via
`g_critical()` and the app keeps running. See `examples/exception_handler.php` and the comments in
`php-gtk.h` / `php-gtk.cpp`, which explain why the handler is a _registered_ value rather than a
name looked up at report time (a pending Zend exception silently no-ops every later `Php::call()`).
Wire any new callback entry point into this path.

## Other

- `docs/reference-objects.md` is the binding-coverage map (legend at the top of the file) — update
  it when a class's status changes. `docs/reference-links.md` collects upstream API links.
- Build/packaging docs: `compile-linux.md`, `compile-windows.md` (+ `compile-windows-old.md`),
  `compile-docker.md`, `compile-mac.md` (legacy), `appimage.md`, `webkit.md`,
  `webview2-windows.md`.
- `gen/` holds a PHP scaffolding generator (`run.php`) driven by C prototypes in `defs.txt`; useful
  for bulk-generating a new class's boilerplate, not part of the build. Generated code is
  hand-edited afterwards — don't regenerate over existing wrappers.
- `.github/copilot-instructions.md` covers the same ground for the Copilot agent; keep the two
  consistent if project-wide conventions change.
- Commit messages follow `fix: …` / `feat: …`.
