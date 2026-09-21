# Compile PHP-GTK3 on Linux

## Index

- [Compile PHP from source](https://github.com/scorninpc/php-gtk3/blob/master/docs/compile-linux.md#compile-php-from-source)
- [Compile PHP-CPP from source](https://github.com/scorninpc/php-gtk3/blob/master/docs/compile-linux.md#compile-php-cpp-from-source)
- [Compile PHP-GTK from source](https://github.com/scorninpc/php-gtk3/blob/master/docs/compile-linux.md#compile-php-gtk3-from-source)
- [Make a manual instalation](https://github.com/scorninpc/php-gtk3/blob/master/docs/compile-linux.md#make-a-manual-instalation)

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

This will create and install libphpcpp.so.2.3

Clone source

```sh
:$ git clone https://github.com/apss-pohl/PHP-CPP.git
:$ cd PHP-CPP
```

`NOTE:` use that fork, not upstream. PHP-CPP registers a module's constants
with `CONST_PERSISTENT`, so the engine releases them with plain `free()` at
module shutdown, but the constructors allocated the strings with request
lifetime (`ZVAL_STRINGL`, i.e. `emalloc`). Freeing those with `free()`
corrupts the heap, and since php-gtk3 declares a lot of string constants,
**every** process loading the built `gtk3.so` aborts on exit:

```
munmap_chunk(): invalid pointer
#1 free ()
#3 zend_hash_destroy ()
#5 php_module_shutdown ()
```

The script still finishes and prints its result, so this is easy to miss -
it shows up as an exit code of 139 on an otherwise successful run. The fork
allocates those strings persistently and releases them in the destructor;
the change is proposed upstream as
[PHP-CPP#557](https://github.com/CopernicaMarketingSoftware/PHP-CPP/pull/557)
and this note can go once it is merged.

Edit `PHP-CPP/Makefile` to use `/opt/php/php-8.2.22/bin/php-config`.

`NOTE:` this path can be changed when you compile PHP-SRC.

Compile and install

```sh
:$ make -j 4 && sudo make install
```

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

If you are compiling from repository version, install `pkg-config` too. If you are compiling from source, use `/usr/local/php-gtk3/bin/php-config` on Makefile

Clone source

```sh
:$ git clone https://github.com/scorninpc/php-gtk3.git PHP-GTK3
:$ cd PHP-GTK3
```

Edit `PHP-CPP/Makefile` to use `/opt/php/php-8.2.22/bin/php-config`.

`NOTE:` this path can be changed when you compile PHP-SRC.

Compile

```sh
:$ make -j 4
```

To compile with WebKit support:

```sh
:$ make WITH_WEBKIT=1 -j 4
```

You can test with

```sh
:$ /opt/php/php-8.2.22/bin/php -dextension=./gtk3.so examples/test1.php
```

## Make a manual instalation

Copy `gtk3.so` created to php library dir

```sh
:$ sudo cp gtk3.so `/usr/local/php-gtk3/bin/php-config --extension-dir`
```

Add `gtk3.so` extension to `/usr/local/php-gtk3/lib/php.ini`. Create the file if needed.

Create a script that will execute all

```sh
:$ sudo nano /usr/bin/php-gtk3
```

With content

```
#!/bin/bash
/opt/php/phpPHP-8.2.22/bin/php -dextension=gtk3.so $@
```

Make it executable

```sh
:$ sudo chmod +x /usr/bin/php-gtk3
```

Done!

```sh
:$ php-gtk3 test1.php
```
