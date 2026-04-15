# OpenGL Support (GtkGLArea)

## Overview

PHP-GTK3 includes OpenGL rendering support through the **GtkGLArea** widget. This widget provides a native OpenGL rendering context that can be used to create 2D and 3D graphics applications.

**Cross-Platform Support:**

- **Linux**: Uses GtkGLArea with libepoxy (OpenGL dispatch library)
- **Windows**: Uses GtkGLArea with libepoxy (provided by GTK3 installation)

The GtkGLArea widget is fully integrated into PHP-GTK3 and works consistently across both platforms.

## Requirements

### Linux Dependencies

On Linux, you need libepoxy for OpenGL dispatch:

```bash
# Ubuntu/Debian
sudo apt-get install libepoxy-dev

# Fedora
sudo dnf install libepoxy-devel

# Arch Linux
sudo pacman -S libepoxy
```

**Note**: libepoxy is usually already installed as a dependency of GTK3.

### Windows Dependencies

On Windows, libepoxy comes with the GTK3 installation:

- `libepoxy-0.dll` - Included in the GTK3 binaries (MSYS2/mingw64)

This DLL is already listed in the Windows deployment documentation and should be copied to your PHP directory along with other GTK3 DLLs.

### OpenGL Graphics Drivers

You also need OpenGL-capable graphics drivers:

- **Linux**: Modern Mesa drivers (usually pre-installed)
- **Windows**: Graphics card drivers from NVIDIA, AMD, or Intel

## Compilation

OpenGL support is built-in and does not require any special compilation flags. The GtkGLArea widget is always available when you build PHP-GTK3:

```bash
make -j 4
sudo make install
```

The Makefile automatically detects your platform and links against the appropriate OpenGL libraries.

## Basic Usage

```php
<?php
Gtk::init();

$window = new GtkWindow();
$window->set_default_size(800, 600);
$window->set_title("OpenGL Example");

// Create a GtkGLArea widget
$glarea = new GtkGLArea();

// Set required OpenGL version (e.g., OpenGL 3.3)
$glarea->set_required_version(3, 3);

// Enable depth buffer for 3D rendering
$glarea->set_has_depth_buffer(true);

// Connect to the 'render' signal
$glarea->connect('render', function($widget, $context) {
    // Make the GL context current
    $widget->make_current();

    // Use OpenGL functions here
    // (requires PHP OpenGL extension or FFI)

    return true;  // Return true on successful render
});

$window->add($glarea);
$window->show_all();

Gtk::main();
```

## Available Methods

### Constructor

- `__construct()` - Create a new GtkGLArea widget

### Context Management

- `make_current()` - Make the GL context current for drawing
- `get_context()` - Get the GdkGLContext (returns pointer as integer)
- `attach_buffers()` - Ensure framebuffer object is complete

### Rendering

- `queue_render()` - Request a render update
- `set_auto_render(bool)` - Enable/disable automatic rendering
- `get_auto_render()` - Check if auto-render is enabled

### Buffer Configuration

- `set_has_alpha(bool)` - Enable/disable alpha channel
- `get_has_alpha()` - Check if alpha channel is enabled
- `set_has_depth_buffer(bool)` - Enable/disable depth buffer
- `get_has_depth_buffer()` - Check if depth buffer is enabled
- `set_has_stencil_buffer(bool)` - Enable/disable stencil buffer
- `get_has_stencil_buffer()` - Check if stencil buffer is enabled

### Version and Error Handling

- `set_required_version(int $major, int $minor)` - Set required OpenGL version
- `get_required_version(&$major, &$minor)` - Get required OpenGL version
- `get_error()` - Get current error message (or null if no error)

## Using OpenGL Functions

The GtkGLArea widget provides the rendering context, but you need to use OpenGL functions to actually draw graphics. There are several ways to do this in PHP:

### Option 1: PHP OpenGL Extension

If you have the PHP OpenGL extension installed, you can use OpenGL functions directly:

```php
$glarea->connect('render', function($widget, $context) {
    $widget->make_current();

    glClearColor(0.2, 0.3, 0.4, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw your geometry here...

    return true;
});
```

### Option 2: FFI (Foreign Function Interface)

Use PHP's FFI to call OpenGL functions:

```php
$gl = FFI::cdef("
    void glClearColor(float r, float g, float b, float a);
    void glClear(unsigned int mask);
    // ... other OpenGL functions
", "libGL.so.1");  // or "opengl32.dll" on Windows

$glarea->connect('render', function($widget, $context) use ($gl) {
    $widget->make_current();

    $gl->glClearColor(0.2, 0.3, 0.4, 1.0);
    $gl->glClear(0x00004000);  // GL_COLOR_BUFFER_BIT

    return true;
});
```

### Option 3: External C Extension

Create a small PHP extension that wraps your OpenGL rendering code.

## Complete Example

See `examples/glarea_example.php` for a complete working example with:

- Window setup
- GtkGLArea configuration
- Signal handling (realize, render, unrealize)
- Interactive controls
- Error handling

## Signals

GtkGLArea emits several important signals:

- **realize** - Called when the widget is realized (initialize GL resources here)
- **render** - Called when the widget needs to be drawn (draw your content here)
- **unrealize** - Called when the widget is destroyed (clean up GL resources here)
- **resize** - Called when the widget size changes

## Best Practices

1. **Initialize in 'realize' signal**: Load shaders, create buffers, and set up GL state in the `realize` signal handler.

2. **Draw in 'render' signal**: Perform all drawing operations in the `render` signal handler.

3. **Cleanup in 'unrealize' signal**: Delete buffers, shaders, and textures in the `unrealize` signal handler.

4. **Always call make_current()**: Before using any OpenGL functions, call `$glarea->make_current()`.

5. **Check for errors**: Use `get_error()` after operations to check for GL context creation errors.

6. **Return true from render**: Always return `true` from the render callback to indicate successful rendering.

## Troubleshooting

### "Failed to create GL context" error

- Ensure your graphics drivers support the requested OpenGL version
- Try reducing the required version: `$glarea->set_required_version(2, 1);`
- Check that your system has OpenGL support: `glxinfo | grep "OpenGL version"` (Linux)

### Black/blank GLArea widget

- Verify you're calling `make_current()` before OpenGL calls
- Check that your render callback returns `true`
- Ensure auto-render is enabled or manually call `queue_render()`

### Windows: "libepoxy-0.dll not found"

- Copy `libepoxy-0.dll` from your MSYS2/mingw64 installation to your PHP directory
- See `docs/compile-windows.md` for the complete list of required DLLs

## Platform-Specific Notes

### Linux

- Works out-of-the-box with modern Mesa drivers
- Hardware acceleration is automatically used when available
- Wayland and X11 are both supported

### Windows

- Requires Windows Vista or newer
- Uses ANGLE (OpenGL ES over DirectX) on some systems
- libepoxy automatically selects the best GL implementation

## References

- [GTK 3 GtkGLArea Documentation](https://developer.gnome.org/gtk3/stable/GtkGLArea.html)
- [libepoxy Documentation](https://github.com/anholt/libepoxy)
- [OpenGL Documentation](https://www.opengl.org/documentation/)
- PHP-GTK3 Example: `examples/glarea_example.php`
