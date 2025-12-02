
# PHP-GTK3

PHP-GTK is a bind of GTK 3 to create desktop applications with PHP

## Features

- 🖥️ **Cross-Platform**: Runs on Linux, Windows, and macOS
- 🎨 **GTK 3 Widgets**: Full access to GTK 3 widget library
- 🌐 **WebKit Support**: Optional WebKitWebView widget for embedding web content (compile with `WITH_WEBKIT=1`)
- 🎯 **GTK Source View**: Code editor widget with syntax highlighting
- 🎪 **Glade Support**: Visual UI designer integration
- 🔄 **PHP-CPP Based**: Modern C++ binding framework
- 📦 **Easy Integration**: Familiar PHP syntax for GTK development

## Documentation

 - [Compile for Windows](https://github.com/scorninpc/php-gtk3/blob/master/docs/compile-windows.md)
 - [Compile for Linux](https://github.com/scorninpc/php-gtk3/blob/master/docs/compile-linux.md)
 - [Compile for macOS](https://github.com/scorninpc/php-gtk3/blob/master/docs/compile-mac.md)
 - [Compile source with Docker](https://github.com/scorninpc/php-gtk3/blob/master/docs/compile-docker.md)
 - [AppImage](https://github.com/scorninpc/php-gtk3/blob/master/docs/appimage.md)
 - [WebKit Support](https://github.com/scorninpc/php-gtk3/blob/master/docs/webkit.md) - Display web content in GTK widgets
 - [Cross-Platform WebView Discussion](https://github.com/scorninpc/php-gtk3/blob/master/docs/webview-library.md) - About using the webview library

## Contributing

Contributions are always welcome!

See [issues](https://github.com/scorninpc/php-gtk3/issues) for problems and improvements.

## How its works

PHP-GTK3 work with a middleway of [PHP-CPP](https://github.com/CopernicaMarketingSoftware/PHP-CPP). In addition to making work faster, syntax and how to program closely resemble PHP, so more people can contribute to the project. Another strong point, perhaps the most important are the updates of Zend.h. In the migration from PHP5 to PHP7 for example, every bind was lost. In that way, there is a nice community working on PHP-CPP to make this updated

## PHP-GTK 3 Cookbook

You can find a cookbook of codes compiled and organized in http://andor.com.br/php-gtk/cookbook

## VSCode

This is [VSCode extension](https://marketplace.visualstudio.com/items?itemName=RobertBeran.PHPGTK3Syntax) (not official) written by [subabrain](https://github.com/subabrain)

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

## FAQ

### Can I use the webview library for cross-platform WebView support?

The [webview library](https://github.com/webview/webview) creates standalone windows and cannot be embedded as GTK widgets. PHP-GTK3's WebKitWebView is designed to be a GTK widget that integrates with the GTK widget hierarchy.

See [docs/webview-library.md](https://github.com/scorninpc/php-gtk3/blob/master/docs/webview-library.md) for a detailed comparison and recommendations.

### How do I add WebView support?

Compile PHP-GTK3 with `WITH_WEBKIT=1`:

```bash
make WITH_WEBKIT=1 -j 4
sudo make install
```

See [docs/webkit.md](https://github.com/scorninpc/php-gtk3/blob/master/docs/webkit.md) for complete WebKit documentation.

### Does PHP-GTK3 work on Windows and macOS?

Yes! GTK 3 can be compiled on Windows and macOS. See the platform-specific compile guides in the documentation section.

## Contributors

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

