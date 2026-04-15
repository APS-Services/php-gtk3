<?php
/**
 * GtkGLArea Example - OpenGL rendering in GTK3
 *
 * This example demonstrates how to use GtkGLArea to render OpenGL content
 * in a PHP-GTK3 application. It works cross-platform on both Linux and Windows.
 *
 * Requirements:
 * - PHP-GTK3 extension with OpenGL support
 * - OpenGL-capable graphics card/driver
 * - Linux: libepoxy (usually installed with GTK3)
 * - Windows: libepoxy-0.dll (deployed with GTK3)
 */

// Create the main window
$window = new GtkWindow();
$window->set_title("PHP-GTK3 OpenGL Example");
$window->set_default_size(800, 600);
$window->connect('destroy', function() {
    Gtk::main_quit();
});

// Create a GtkGLArea widget
$glarea = new GtkGLArea();

// Set OpenGL version requirement (OpenGL 3.3)
$glarea->set_required_version(3, 3);

// Enable alpha channel for transparency (optional)
$glarea->set_has_alpha(true);

// Enable depth buffer for 3D rendering
$glarea->set_has_depth_buffer(true);

// Enable stencil buffer (optional, for advanced rendering)
$glarea->set_has_stencil_buffer(false);

// Auto-render mode - automatically triggers render signal
$glarea->set_auto_render(true);

// Connect to the 'render' signal to draw OpenGL content
$glarea->connect('render', function($widget, $context) {
    // Make the GL context current
    $widget->make_current();

    // Note: In a real application, you would use OpenGL functions here
    // For example, using PHP's OpenGL extension or FFI to call OpenGL functions:
    //
    // glClearColor(0.2, 0.3, 0.4, 1.0);  // Dark blue background
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //
    // // Draw a triangle, cube, or other 3D geometry
    // glBegin(GL_TRIANGLES);
    // glColor3f(1.0, 0.0, 0.0); glVertex3f( 0.0,  1.0, 0.0);
    // glColor3f(0.0, 1.0, 0.0); glVertex3f(-1.0, -1.0, 0.0);
    // glColor3f(0.0, 0.0, 1.0); glVertex3f( 1.0, -1.0, 0.0);
    // glEnd();
    //
    // glFlush();

    echo "Render callback triggered - GL context: " . $widget->get_context() . "\n";

    // Return true to indicate successful render
    return true;
});

// Connect to 'realize' signal - called when widget is realized
$glarea->connect('realize', function($widget) {
    echo "GtkGLArea realized\n";

    // Make GL context current
    $widget->make_current();

    // Check for errors
    $error = $widget->get_error();
    if ($error !== null) {
        echo "GL Error: " . $error . "\n";
        return;
    }

    // Get required OpenGL version
    $major = 0;
    $minor = 0;
    $widget->get_required_version($major, $minor);
    echo "Required OpenGL version: " . $major . "." . $minor . "\n";

    // Print GL area properties
    echo "Has alpha: " . ($widget->get_has_alpha() ? "yes" : "no") . "\n";
    echo "Has depth buffer: " . ($widget->get_has_depth_buffer() ? "yes" : "no") . "\n";
    echo "Has stencil buffer: " . ($widget->get_has_stencil_buffer() ? "yes" : "no") . "\n";
    echo "Auto render: " . ($widget->get_auto_render() ? "yes" : "no") . "\n";

    // Initialize OpenGL state here
    // This is where you would typically:
    // - Load shaders
    // - Create vertex buffers
    // - Set up textures
    // - Configure OpenGL state
});

// Connect to 'unrealize' signal - cleanup
$glarea->connect('unrealize', function($widget) {
    echo "GtkGLArea unrealized - cleanup GL resources\n";

    // Make GL context current for cleanup
    $widget->make_current();

    // Clean up OpenGL resources here
    // - Delete buffers
    // - Delete shaders
    // - Delete textures
});

// Create a box to hold additional controls
$vbox = new GtkBox(GtkOrientation::VERTICAL, 5);
$vbox->set_margin_start(10);
$vbox->set_margin_end(10);
$vbox->set_margin_top(10);
$vbox->set_margin_bottom(10);

// Add the GL area (it will expand to fill available space)
$vbox->pack_start($glarea, true, true, 0);

// Add a button to trigger manual render
$render_button = new GtkButton("Trigger Render");
$render_button->connect('clicked', function() use ($glarea) {
    echo "Manually triggering render...\n";
    $glarea->queue_render();
});
$vbox->pack_start($render_button, false, false, 5);

// Add a button to print GL info
$info_button = new GtkButton("Print GL Info");
$info_button->connect('clicked', function() use ($glarea) {
    echo "\n=== GL Area Information ===\n";
    echo "GL Context: " . $glarea->get_context() . "\n";

    $major = 0;
    $minor = 0;
    $glarea->get_required_version($major, $minor);
    echo "Required GL version: " . $major . "." . $minor . "\n";

    $error = $glarea->get_error();
    echo "Current error: " . ($error !== null ? $error : "none") . "\n";
    echo "==========================\n\n";
});
$vbox->pack_start($info_button, false, false, 5);

// Add instructions label
$label = new GtkLabel();
$label->set_markup(
    "<b>GtkGLArea Example</b>\n\n" .
    "This widget provides an OpenGL rendering context.\n" .
    "To see actual 3D graphics, you would need to use OpenGL functions\n" .
    "via PHP's OpenGL extension or FFI.\n\n" .
    "The 'render' signal is triggered when the widget needs to be redrawn."
);
$label->set_line_wrap(true);
$vbox->pack_start($label, false, false, 10);

// Add the box to the window
$window->add($vbox);

// Show all widgets
$window->show_all();

echo "GtkGLArea example started.\n";
echo "Check the console for render callbacks and GL information.\n\n";

// Run the main GTK loop
Gtk::main();
