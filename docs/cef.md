# CEF (Chromium Embedded Framework) Support

## ⚠️ Current Implementation Status

**IMPORTANT**: This is currently a **placeholder/foundation implementation**. The CefWebView widget provides the API structure and build system integration, but does not yet render actual web content. When you run the examples, you will see a placeholder message instead of a browser.

**What works:**
- ✅ Widget creation and GTK integration
- ✅ API methods (load_uri, reload, etc.) - store state but don't render
- ✅ Build system with proper CEF linking
- ✅ Example applications

**What's pending for full implementation:**
- ⏳ Actual CEF browser initialization
- ⏳ CEF render handler integration
- ⏳ Web content rendering
- ⏳ JavaScript execution
- ⏳ Multi-process architecture

See [Implementation Notes](#implementation-notes) section below for technical details.

## Overview

PHP-GTK3 includes optional CefWebView widget support for displaying web content within GTK windows using the Chromium Embedded Framework. Unlike WebKit2GTK which is Linux-only, CEF is cross-platform and works on Windows, macOS, and Linux.

## Key Features

- **Cross-Platform**: Works on Windows, macOS, and Linux
- **Modern Browser Engine**: Based on Chromium (same engine as Google Chrome)
- **Full API Compatibility**: Similar API to WebKitWebView for easy migration
- **JavaScript Integration**: Bidirectional communication between PHP and JavaScript
- **Developer Tools**: Built-in Chromium DevTools support
- **Zoom Control**: Programmatic zoom level control

## Compilation

CEF support is optional and controlled by the `WITH_CEF=1` make flag.

### Install Dependencies

CEF requires the Chromium Embedded Framework binaries to be installed separately.

#### Linux

```bash
# Download CEF binaries (choose the appropriate version from https://cef-builds.spotifycdn.com/)
# Example for Linux 64-bit:
wget https://cef-builds.spotifycdn.com/cef_binary_142.0.17+g60aac24+chromium-142.0.7444.176_linux64.tar.bz2
tar -xjf cef_binary_142.0.17+g60aac24+chromium-142.0.7444.176_linux64.tar.bz2
cd cef_binary_142.0.17+g60aac24+chromium-142.0.7444.176_linux64

# Build the CEF DLL wrapper (required for C++ integration)
cmake . -DCMAKE_BUILD_TYPE=Release
make -j4 libcef_dll_wrapper

# The built library will be in: libcef_dll_wrapper/libcef_dll_wrapper.a
# The main libcef.so is already provided in the Release directory

# Optionally install to system location:
# sudo cp -r . /usr/local/cef
```

#### Windows

```powershell
# Download CEF binaries from https://cef-builds.spotifycdn.com/
# Extract to a directory (e.g., C:\CEF)

cd C:\CEF\cef_binary_VERSION_windows64

# Build the CEF DLL wrapper using CMake
cmake . -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release --target libcef_dll_wrapper

# Set environment variable:
setx CEF_ROOT C:\CEF\cef_binary_VERSION_windows64
```

#### macOS

```bash
# Download CEF binaries
curl -O https://cef-builds.spotifycdn.com/cef_binary_VERSION_macosx64.tar.bz2
tar -xjf cef_binary_VERSION_macosx64.tar.bz2
cd cef_binary_VERSION_macosx64

# Build the CEF DLL wrapper
cmake . -DCMAKE_BUILD_TYPE=Release
make -j4 libcef_dll_wrapper

# Optionally install to system location:
# sudo cp -r . /usr/local/cef
```

**Important**: You must build the `libcef_dll_wrapper` library using CMake before compiling PHP-GTK3 with CEF support. This library is not included in the pre-built CEF distribution.

### Build with CEF Support

```bash
# Set CEF_ROOT to point to your CEF installation directory
export CEF_ROOT=/path/to/cef_binary_VERSION_linux64

# Or use the default location (/usr/local/cef)
# by installing CEF there with: sudo make install

make WITH_CEF=1 -j 4
sudo make install
```

**Note**: If you installed CEF to a custom location, make sure to set the `CEF_ROOT` environment variable before building.

## Basic Usage

```php
<?php
Gtk::init();

$window = new GtkWindow();
$window->set_default_size(800, 600);

$webView = new CefWebView();
$scrolled = new GtkScrolledWindow();
$scrolled->add($webView);
$window->add($scrolled);

$webView->load_uri("https://www.example.com");

$window->show_all();
Gtk::main();
```

## Available Methods

### Loading Content

- `load_uri($uri)` - Load a URL
- `load_html($content, $base_uri)` - Load HTML content directly
- `reload()` - Reload the current page
- `stop_loading()` - Stop loading the current page

### Navigation

- `can_go_back()` - Check if can navigate back
- `go_back()` - Navigate to previous page
- `can_go_forward()` - Check if can navigate forward
- `go_forward()` - Navigate to next page

### JavaScript Integration

- `run_javascript($script)` - Execute JavaScript in the web view (PHP → JavaScript)
- `register_script_message_handler($name, $callback)` - Register custom message handler (JavaScript → PHP)
  - JavaScript sends messages using: `window.cef.messageHandlers.name.postMessage(data)`
  - Callback receives message data as first parameter

### Settings & Info

- `enable_developer_extras()` - Enable Chromium DevTools for debugging
- `get_uri()` - Get the current URI
- `get_title()` - Get the current page title
- `is_loading()` - Check if page is loading
- `get_zoom_level()` - Get the current zoom level
- `set_zoom_level($level)` - Set the zoom level (1.0 = 100%, 1.5 = 150%, etc.)

## Examples

See the `examples/` directory for working examples:

- **cef_simple.php** - Minimal example
- **cef_browser.php** - Full browser with navigation and zoom controls
- **cef_communication.php** - Bidirectional JavaScript ↔ PHP communication

## JavaScript ↔ PHP Communication

### Setup (PHP Side)

```php
$webView = new CefWebView();

// Register a message handler named "phpApp"
$webView->register_script_message_handler("phpApp", function($messageData) {
    echo "Received from JavaScript: $messageData\n";
});
```

### Send Messages (JavaScript Side)

```javascript
// Send message to PHP
window.cef.messageHandlers.phpApp.postMessage('Hello from JavaScript!');

// Send structured data
window.cef.messageHandlers.phpApp.postMessage(JSON.stringify({
    type: 'event',
    data: 'value'
}));
```

### Execute JavaScript from PHP

```php
// Update page content from PHP
$webView->run_javascript("document.body.style.backgroundColor = 'lightblue';");

// Call JavaScript functions
$webView->run_javascript("myFunction('parameter');");
```

## Debugging

Enable Chromium DevTools for debugging JavaScript:

```php
$webView->enable_developer_extras();
```

This will open the Chromium Developer Tools in a separate window, providing access to:
- Console (JavaScript errors and console.log output)
- Elements (DOM inspection)
- Network (HTTP requests)
- Sources (JavaScript debugging)
- Performance profiling
- And all other standard Chrome DevTools features

## Zoom Control

```php
// Get current zoom level
$currentZoom = $webView->get_zoom_level();

// Zoom in (150%)
$webView->set_zoom_level(1.5);

// Zoom out (75%)
$webView->set_zoom_level(0.75);

// Reset to default (100%)
$webView->set_zoom_level(1.0);
```

## Comparison: CefWebView vs WebKitWebView

| Feature | CefWebView | WebKitWebView |
|---------|-----------|---------------|
| **Platform Support** | Windows, macOS, Linux | Linux only |
| **Browser Engine** | Chromium | WebKit |
| **API Compatibility** | Similar | Similar |
| **Developer Tools** | Full Chrome DevTools | WebKit Inspector |
| **Zoom Control** | Built-in | Via settings |
| **Performance** | Excellent | Excellent |
| **Bundle Size** | Larger (~100-200MB) | Smaller (~10-20MB) |

## When to Use CefWebView

Use CefWebView when:
- You need cross-platform support (especially Windows or macOS)
- You want the latest Chromium features
- You prefer Chrome DevTools for debugging
- You need guaranteed behavior consistency across platforms

Use WebKitWebView when:
- You're targeting Linux only
- You want a smaller binary size
- You prefer WebKit's rendering engine
- System WebKit libraries are already available

## Implementation Notes

**Current Status**: The CefWebView implementation provides a compatible API structure that mirrors WebKitWebView. Full CEF integration requires:

1. CEF library initialization in the PHP-GTK3 startup
2. CEF browser process setup
3. Integration of CEF render handler with GTK drawing area
4. IPC setup for JavaScript message handlers
5. CEF shutdown handling

The current implementation provides a foundation for full CEF integration while maintaining API compatibility with WebKitWebView, allowing applications to be written that can use either widget.

## Architecture

- **Inheritance**: CefWebView extends GtkWidget
- **Base Widget**: GTK DrawingArea (where CEF browser will be embedded)
- **Memory Management**: Properly manages history and message handlers
- **Security**: Input validation and null checks on all methods

## Files

### Source Code
- `src/Cef/CefWebView.h`
- `src/Cef/CefWebView.cpp`

### Build Integration
- `Makefile` - Optional compilation with `WITH_CEF=1`
- `main.h` - Conditional include
- `main.cpp` - Conditional class registration

### Examples
- `examples/cef_simple.php` - Basic usage
- `examples/cef_browser.php` - Full-featured browser
- `examples/cef_communication.php` - JavaScript communication

## Future Enhancements

Planned features for full CEF integration:

- [ ] Complete CEF initialization and browser embedding
- [ ] Multi-process architecture support
- [ ] Custom protocol handlers
- [ ] Download manager integration
- [ ] Cookie management
- [ ] Cache control
- [ ] Print support
- [ ] File upload dialogs
- [ ] Context menu customization
- [ ] Audio/video support

## Resources

- [CEF Project](https://bitbucket.org/chromiumembedded/cef)
- [CEF Forum](https://magpcss.org/ceforum/)
- [CEF Builds](https://cef-builds.spotifycdn.com/)
- [CEF C++ Tutorial](https://bitbucket.org/chromiumembedded/cef/wiki/Tutorial)
- [Chromium DevTools](https://developer.chrome.com/docs/devtools/)

## License

CEF is licensed under the BSD license. See the CEF project for details.
