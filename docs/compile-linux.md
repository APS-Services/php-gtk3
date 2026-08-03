# Compile PHP-GTK3 on Linux

## Index

- [Compile PHP from source](https://github.com/scorninpc/php-gtk3/blob/master/docs/compile-linux.md#compile-php-from-source)
- [Compile PHP-CPP from source](https://github.com/scorninpc/php-gtk3/blob/master/docs/compile-linux.md#compile-php-cpp-from-source)
- [Compile PHP-GTK from source](https://github.com/scorninpc/php-gtk3/blob/master/docs/compile-linux.md#compile-php-gtk3-from-source)
- [Make a manual instalation](https://github.com/scorninpc/php-gtk3/blob/master/docs/compile-linux.md#make-a-manual-instalation)

## Which PHP to build against

Two options. Whichever you pick, **PHP-CPP and php-gtk3 must be built against the same
`php-config`.** Nothing checks this, and getting it wrong does not fail the build: the resulting
`gtk3.so` loads and runs, then corrupts the heap during PHP's module shutdown, so the process aborts
with `free(): invalid pointer` *after* your script has already finished. `ini_get('gtk3.phpcpp')`
reports which libphpcpp a built binary actually linked.

**Use your distribution's PHP** (simplest — skip the next section entirely):

```sh
:$ sudo apt-get install php8.4-dev        # provides /usr/bin/php-config8.4
```

Then pass `PHP_CONFIG=/usr/bin/php-config8.4` to both builds. This is what `buildall.sh` and
`PHP-CPP/build-dist.sh` do, and it is the layout the Makefile defaults to.

**Or compile PHP from source**, if you need a version or build options your distro does not ship.
That is what the next section covers.

## Compile PHP from source

Install dependencies for php and extensions. On deb packages for example:

```sh
:$ sudo apt-get install libpq-dev bison libreadline-dev git build-essential autoconf automake libtool re2c libxml2-dev libcurl4-openssl-dev libssl-dev libbz2-dev  libjpeg-dev libpng-dev libxpm-dev libfreetype6-dev libzip-dev libsqlite3-dev libonig-dev libxslt1-dev
```

Clone php souce

```sh
:$ git clone -b PHP-8.2.22 https://github.com/php/php-src.git PHP-8.2.22
:$ cd PHP-8.2.22
```

Build the config

```sh
:$ ./buildconf --force
```

Configure

```sh
:$ ./configure --prefix=/opt/php/php-8.2.22 --disable-cgi --disable-shared --enable-static --with-pdo-mysql --with-pdo-pgsql --with-ffi --with-readline --with-openssl --enable-soap --enable-sockets --with-bz2 --with-zlib --enable-mbstring --with-curl --with-xsl
```

Compile and install

```sh
:$ make -j 4 && sudo make install
```

## Compile PHP-CPP from source

This creates `libphpcpp.so` / `libphpcpp.a`, the Zend-API abstraction php-gtk3 links against.

Clone source

```sh
:$ git clone https://github.com/fast-debug/PHP-CPP.git
:$ cd PHP-CPP
```

Compile and install against a specific PHP. `PHP_VERSION=8.4` selects `php-config8.4`; use
`PHP_CONFIG=/full/path/to/php-config` if yours is not on `$PATH` under that name:

```sh
:$ make release PHP_VERSION=8.4 -j 4 && sudo make install
```

**Remember this choice** — the next section has to use the same one. Building php-gtk3 against a
different PHP than this produces a binary that aborts at shutdown rather than failing to build.

## Compile PHP-GTK3 from source

This will create gtk3.so php module

Get GTK dependencies, on deb systems, like:

```sh
:$ sudo apt-get install libgtk-3-dev libgladeui-dev libgtksourceview-3.0-dev libwnck-3-dev
```

For WebKit support (optional), also install:

```sh
:$ sudo apt-get install libwebkit2gtk-4.1-dev
```

If you are compiling from repository version, install `pkg-config` too.

Clone source

```sh
:$ git clone https://github.com/scorninpc/php-gtk3.git PHP-GTK3
:$ cd PHP-GTK3
```

Compile, pointing `PHP_CONFIG` at **the same `php-config` you built PHP-CPP with**:

```sh
:$ make PHP_CONFIG=/usr/bin/php-config8.4 -j 4
```

The Makefile defaults to `/usr/bin/php-config8.4` and falls back to `php-config` from `$PATH`, so
plain `make` works if that is already the PHP you want. If you compiled PHP from source, use its
`php-config` here instead — e.g. `/opt/php/php-8.2.22/bin/php-config`.

To compile with WebKit support:

```sh
:$ make WITH_WEBKIT=1 -j 4
```

You can test with

```sh
:$ php8.4 -dextension=./gtk3.so examples/exception_handler.php
```

Loading the freshly built `.so` this way, rather than installing it first, keeps a bad build from
disturbing a working installation.

## Make a manual instalation

`make install` does both steps below. To do it by hand instead:

Copy `gtk3.so` to the extension dir of the PHP you built against — always ask that PHP's own
`php-config`, so the file cannot land in the wrong version's directory:

```sh
:$ sudo cp gtk3.so `/usr/bin/php-config8.4 --extension-dir`
```

Enable it. On Debian/Ubuntu that means a file in the `conf.d` of each SAPI you want it in:

```sh
:$ echo 'extension=gtk3' | sudo tee /etc/php/8.4/cli/conf.d/20-gtk3.ini
```

On installations with a single `php.ini`, add `extension=gtk3` there instead.

Check it loaded — the name reported here is `gtk3`, matching the ini directive:

```sh
:$ php8.4 -m | grep gtk3
:$ php8.4 -r 'echo ini_get("gtk3.build_info"), "\n";'
```

Create a script that will execute all

```sh
:$ sudo nano /usr/bin/php-gtk3
```

With content

```
#!/bin/bash
php8.4 -dextension=gtk3.so "$@"
```

Make it executable

```sh
:$ sudo chmod +x /usr/bin/php-gtk3
```

Done!

```sh
:$ php-gtk3 examples/exception_handler.php
```
