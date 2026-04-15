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

### OpenGL Support (GtkGLArea)

PHP-GTK3 includes built-in OpenGL rendering support through the **GtkGLArea** widget:

- **Cross-platform**: Works on both Linux and Windows
- **Hardware-accelerated**: Uses native OpenGL drivers
- **Full GTK integration**: Seamlessly integrates with other GTK widgets

```php
<?php
Gtk::init();

$window = new GtkWindow();
$glarea = new GtkGLArea();

// Set OpenGL version (e.g., OpenGL 3.3)
$glarea->set_required_version(3, 3);
$glarea->set_has_depth_buffer(true);

// Render callback
$glarea->connect('render', function($widget) {
    $widget->make_current();
    // Use OpenGL functions here (via FFI or PHP OpenGL extension)
    return true;
});

$window->add($glarea);
$window->show_all();
Gtk::main();
```

See [OpenGL Documentation](docs/opengl.md) for full details and examples.

**Platform implementations:**

- Linux: libepoxy (OpenGL dispatch library)
- Windows: libepoxy (included with GTK3)

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
