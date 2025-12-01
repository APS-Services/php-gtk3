# PHP-GTK3 Examples

This directory contains examples demonstrating the use of PHP-GTK3 widgets and features.

## WebKit Examples

### webkit_simple.php

A minimal example showing how to display a web page in a GTK window using WebKitWebView.

**Requirements:**

- PHP-GTK3 compiled with `WITH_WEBKIT=1`
- webkit2gtk-4.1 library installed

**Usage:**

```bash
php -dextension=php-gtk3.so webkit_simple.php
```

### webkit_browser.php

A more complete example with navigation controls (back, forward, reload) and URL entry.

**Requirements:**

- PHP-GTK3 compiled with `WITH_WEBKIT=1`
- webkit2gtk-4.1 library installed

**Usage:**

```bash
php -dextension=php-gtk3.so webkit_browser.php
```

### webkit_js_to_php.php

Demonstrates bidirectional communication between JavaScript and PHP.

**Features:**

- Loading static HTML content with `load_html()`
- Executing JavaScript from PHP with `run_javascript()`
- Register custom message handler with `register_script_message_handler()`
- JavaScript sends messages to PHP using `window.webkit.messageHandlers`
- Full bidirectional PHP ↔ JavaScript communication with payload data

**Requirements:**

- PHP-GTK3 compiled with `WITH_WEBKIT=1`
- webkit2gtk-4.1 library installed

**Usage:**

```bash
php -dextension=php-gtk3.so webkit_js_to_php.php
```

## Compiling PHP-GTK3 with WebKit Support

To use the WebKit examples, you need to compile PHP-GTK3 with WebKit support:

1. Install WebKit2GTK development libraries:

   ```bash
   sudo apt-get install libwebkit2gtk-4.1-dev
   ```

2. Compile PHP-GTK3 with WebKit support:

   ```bash
   make WITH_WEBKIT=1 -j 4
   ```

3. Install the extension:
   ```bash
   sudo make install
   ```

For more information, see the [compilation documentation](../docs/compile-linux.md).
