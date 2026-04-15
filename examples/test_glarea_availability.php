<?php
/**
 * Test script to check GtkGLArea availability and OpenGL support
 *
 * This script checks if:
 * 1. GtkGLArea class is available
 * 2. GTK version meets requirements
 * 3. A simple GtkGLArea widget can be created
 * 4. Basic OpenGL context information
 */

echo "=== GtkGLArea Availability Test ===\n\n";

// Test 1: Check if GTK is initialized
echo "1. Initializing GTK...\n";
try {
    Gtk::init();
    echo "   ✓ GTK initialized successfully\n\n";
} catch (Exception $e) {
    echo "   ✗ Failed to initialize GTK: " . $e->getMessage() . "\n";
    exit(1);
}

// Test 2: Check GTK version
echo "2. Checking GTK version...\n";
$gtk_major = Gtk::get_major_version();
$gtk_minor = Gtk::get_minor_version();
$gtk_micro = Gtk::get_micro_version();
echo "   GTK Version: $gtk_major.$gtk_minor.$gtk_micro\n";

if ($gtk_major < 3 || ($gtk_major == 3 && $gtk_minor < 16)) {
    echo "   ✗ GTK version too old. GtkGLArea requires GTK 3.16+\n";
    echo "   Your version: $gtk_major.$gtk_minor.$gtk_micro\n";
    exit(1);
} else {
    echo "   ✓ GTK version meets requirements (3.16+)\n\n";
}

// Test 3: Check if GtkGLArea class exists
echo "3. Checking if GtkGLArea class is available...\n";
if (!class_exists('GtkGLArea')) {
    echo "   ✗ GtkGLArea class not found!\n";
    echo "   \n";
    echo "   Possible reasons:\n";
    echo "   - PHP-GTK3 extension was compiled with GTK < 3.16\n";
    echo "   - Extension not loaded properly\n";
    echo "   - Missing dependencies (libepoxy)\n";
    echo "   \n";
    echo "   Windows specific checks:\n";
    echo "   - Ensure libepoxy-0.dll is in your PHP directory\n";
    echo "   - Verify GTK3 was built with OpenGL support\n";
    echo "   - Rebuild PHP-GTK3 extension after updating GTK\n";
    exit(1);
}
echo "   ✓ GtkGLArea class is available\n\n";

// Test 4: Try to create a GtkGLArea widget
echo "4. Creating GtkGLArea widget...\n";
try {
    $glarea = new GtkGLArea();
    echo "   ✓ GtkGLArea widget created successfully\n\n";
} catch (Exception $e) {
    echo "   ✗ Failed to create GtkGLArea: " . $e->getMessage() . "\n";
    exit(1);
}

// Test 5: Check available methods
echo "5. Checking available methods...\n";
$methods = get_class_methods('GtkGLArea');
$expected_methods = [
    'make_current',
    'queue_render',
    'attach_buffers',
    'get_context',
    'set_has_alpha',
    'get_has_alpha',
    'set_has_depth_buffer',
    'get_has_depth_buffer',
    'set_auto_render',
    'get_auto_render',
    'set_required_version',
    'get_required_version',
    'get_error'
];

$missing = [];
foreach ($expected_methods as $method) {
    if (!in_array($method, $methods)) {
        $missing[] = $method;
    }
}

if (!empty($missing)) {
    echo "   ⚠ Some methods are missing: " . implode(', ', $missing) . "\n\n";
} else {
    echo "   ✓ All expected methods are available\n\n";
}

// Test 6: Create a window and realize the widget to test OpenGL context
echo "6. Testing OpenGL context creation...\n";
$window = new GtkWindow();
$window->set_default_size(400, 300);
$window->set_title("GtkGLArea Test");

// Set a reasonable OpenGL version
$glarea->set_required_version(2, 1);  // OpenGL 2.1 (widely supported)
$glarea->set_has_depth_buffer(true);

$context_created = false;
$error_message = null;

// Connect to realize signal to test context creation
$glarea->connect('realize', function($widget) use (&$context_created, &$error_message) {
    echo "   Widget realized, checking GL context...\n";

    // Make current and check for errors
    $widget->make_current();

    $error = $widget->get_error();
    if ($error !== null) {
        $error_message = $error;
        return;
    }

    $context = $widget->get_context();
    if ($context > 0) {
        echo "   ✓ GL context created: " . $context . "\n";
        $context_created = true;

        // Get the OpenGL version that was set
        $major = 0;
        $minor = 0;
        $widget->get_required_version($major, $minor);
        echo "   Required GL version: $major.$minor\n";

        // Check properties
        echo "   Has alpha: " . ($widget->get_has_alpha() ? "yes" : "no") . "\n";
        echo "   Has depth buffer: " . ($widget->get_has_depth_buffer() ? "yes" : "no") . "\n";
        echo "   Auto render: " . ($widget->get_auto_render() ? "yes" : "no") . "\n";
    } else {
        $error_message = "Context is null or invalid";
    }
});

$window->add($glarea);
$window->show_all();

// Process events to trigger realize
while (Gtk::events_pending()) {
    Gtk::main_iteration();
}

// Give it a moment to realize
usleep(100000);  // 100ms

if ($context_created) {
    echo "   ✓ OpenGL context creation successful!\n\n";
} else {
    echo "   ✗ Failed to create OpenGL context\n";
    if ($error_message) {
        echo "   Error: $error_message\n";
    }
    echo "   \n";
    echo "   This could mean:\n";
    echo "   - Missing OpenGL drivers\n";
    echo "   - OpenGL not supported on this system\n";
    echo "   - Graphics drivers need updating\n\n";
}

// Clean up
$window->destroy();

// Final summary
echo "=== Summary ===\n";
echo "GTK Version: $gtk_major.$gtk_minor.$gtk_micro\n";
echo "GtkGLArea Available: " . (class_exists('GtkGLArea') ? "YES" : "NO") . "\n";
echo "Widget Creation: " . (isset($glarea) ? "SUCCESS" : "FAILED") . "\n";
echo "GL Context: " . ($context_created ? "SUCCESS" : "FAILED") . "\n";

if (class_exists('GtkGLArea') && $context_created) {
    echo "\n✓ All tests passed! GtkGLArea is working correctly.\n";
    exit(0);
} else {
    echo "\n✗ Some tests failed. See output above for details.\n";
    exit(1);
}
