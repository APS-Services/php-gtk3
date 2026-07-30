# PHP-GTK3

PHP-GTK is a bind of GTK 3 to create desktop applications with PHP

## Documentation

- [Compile for window](https://github.com/scorninpc/php-gtk3/blob/master/docs/compile-windows.md)
- [Compile for linux](https://github.com/scorninpc/php-gtk3/blob/master/docs/compile-linux.md)
- [Compile for mac](https://github.com/scorninpc/php-gtk3/blob/master/docs/compile-mac.md)
- [Compile source with docker](https://github.com/scorninpc/php-gtk3/blob/master/docs/compile-docker.md)
- [Appimage](https://github.com/scorninpc/php-gtk3/blob/master/docs/appimage.md)

## Contributing

Contributions are always welcome!

See [issues](https://github.com/scorninpc/php-gtk3/issues) for problems and improvements.

## How its works

PHP-GTK3 work with a middleway of [PHP-CPP](https://github.com/CopernicaMarketingSoftware/PHP-CPP). In addition to making work faster, syntax and how to program closely resemble PHP, so more people can contribute to the project. Another strong point, perhaps the most important are the updates of Zend.h. In the migration from PHP5 to PHP7 for example, every bind was lost. In that way, there is a nice community working on PHP-CPP to make this updated

## PHP-GTK 3 Cookbook

You can find a cookbook of codes compiled and organized in <http://andor.com.br/php-gtk/cookbook>

## VSCode

This is [VSCode extension](https://marketplace.visualstudio.com/items?itemName=RobertBeran.PHPGTK3Syntax) (not official) written by [subabrain](https://github.com/subabrain)

## Features

### WebView Support (Cross-Platform)

PHP-GTK3 includes optional WebView widget support for displaying web content within GTK windows:

- **WebKitWebView** - Cross-platform web browser widget
- **GtkWebView** - Convenient alias with GTK-style naming

**Platform implementations:**

- Linux/macOS: WebKit2GTK
- Windows: Microsoft Edge WebView2

```php
<?php
Gtk::init();

$window = new GtkWindow();
$webView = new GtkWebView();  // or new WebKitWebView()
$webView->load_uri("https://www.example.com");

$scrolled = new GtkScrolledWindow();
$scrolled->add($webView);
$window->add($scrolled);

$window->show_all();
Gtk::main();
```

See [WebKit Documentation](docs/webkit.md) for full details and examples.

To compile with WebKit support: `make WITH_WEBKIT=1`

### System Tray Icons (GtkStatusIcon)

PHP-GTK3 includes `GtkStatusIcon` for creating system tray icons:

**Platform Support:**

- ✅ **Windows** - Full support using native system tray
- ✅ **Linux (X11)** - Full support
- ⚠️ **Linux (Wayland)** - **Not supported**

**Quick Workaround for Wayland:**

```bash
# Force X11 compatibility mode
GDK_BACKEND=x11 php your_app.php
```

### Exception Handling in Callbacks

An exception thrown inside a signal handler (or other callback, like a `Gtk::timeout_add` function) cannot propagate out of GTK's main loop, so a `try/catch` around `Gtk::main()` will **not** see it. Instead, PHP-GTK3 catches it at the C++/PHP boundary and reports it - by default with a `g_critical()` message on stderr, and the application keeps running.

To route these failures into your own logging or error dialog, install a handler:

```php
<?php
Gtk::set_exception_handler(function (string $message, string $origin, int $code) {
    // $origin is the signal name, or the installing method (e.g. "Gtk::timeout_add")
    error_log("[$origin] handler failed: $message");
});
```

Pass `null` to remove the handler again. Both PHP `Exception`s and `Error`s (e.g. a `TypeError` or an undefined constant) are reported this way. Note that only the message and code are available - the original `Throwable` object (class, file, line, trace) cannot cross the C boundary, so catch inside your own callback if you need those details.

See [examples/exception_handler.php](examples/exception_handler.php) for a runnable example.

## Example

```php
<?php

// Initialize
Gtk::init();

// Callback for when window is closed
function GtkWindowDestroy($widget=NULL, $event=NULL)
{
 Gtk::main_quit();
}

// Create a window
$win = new GtkWindow();
$win->set_default_size(300, 200);

// Connect "close" event with callback
$win->connect("destroy", "GtkWindowDestroy");

// Show window
$win->show_all();

// Start
Gtk::main();
```

## Involved

- [@scorninpc](https://www.github.com/scorninpc) - Main dev
- [@subabrain](https://www.github.com/subabrain) - Thank you for your hard work to compile on Windows
- [@apss-pohl](https://github.com/apss-pohl) - A lot fixes and improvements

## Screenshots

![GTKTreeview](https://i.imgur.com/zrO87QV.jpg)

![GTKTreeview](https://i.imgur.com/NeEjyLj.jpg)

![GTK interactive debugger](https://i.imgur.com/P50OpKF.jpg)

![GTK interactive debugger](https://i.imgur.com/NZkie9R.jpg)

![Widgets](https://i.imgur.com/UpBIYQB.jpg)

![Tooltips](https://i.imgur.com/cGECrWu.jpg)
