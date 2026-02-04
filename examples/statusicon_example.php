<?php
/**
 * GtkStatusIcon Example
 * 
 * This example demonstrates how to create a status icon in the system tray.
 * Note: GtkStatusIcon is deprecated since GTK 3.14, but still functional.
 * 
 * GTK is now automatically initialized when creating a GtkStatusIcon, so you
 * don't need to call Gtk::init() manually (though you still can if you need
 * command-line argument processing).
 * 
 * Two fixes have been applied:
 * 1. Automatic GTK initialization prevents crashes when GTK isn't initialized
 * 2. Log handler suppression prevents the gtk_widget_get_scale_factor warning
 * 
 * These fixes apply to all GtkStatusIcon creation and update methods.
 */

// Create a simple test icon file
$icon_file = '/tmp/test_icon.png';
if (!file_exists($icon_file)) {
    // Create a simple 48x48 PNG icon with true color support
    $image = imagecreatetruecolor(48, 48);
    $bg = imagecolorallocate($image, 255, 0, 0); // Red background
    $fg = imagecolorallocate($image, 255, 255, 255); // White foreground
    imagefilledrectangle($image, 0, 0, 48, 48, $bg);
    imagefilledellipse($image, 24, 24, 30, 30, $fg);
    imagepng($image, $icon_file);
    imagedestroy($image);
}

// Example 1: Create status icon from file
echo "Creating status icon from file...\n";
$status_icon = GtkStatusIcon::new_from_file($icon_file);
$status_icon->set_tooltip_text("Status Icon Example");
$status_icon->set_visible(true);

// Example 2: Create status icon from stock icon
echo "Creating status icon from stock icon...\n";
$status_icon2 = GtkStatusIcon::new_from_stock(Gtk::STOCK_INFO);
$status_icon2->set_tooltip_text("Stock Icon Example");
$status_icon2->set_visible(true);

// Example 3: Create status icon from icon name
echo "Creating status icon from icon name...\n";
$status_icon3 = GtkStatusIcon::new_from_icon_name("dialog-information");
$status_icon3->set_tooltip_text("Icon Name Example");
$status_icon3->set_visible(true);

// Example 4: Create empty status icon and set image later
echo "Creating empty status icon and setting image...\n";
$status_icon4 = new GtkStatusIcon();
$status_icon4->set_from_file($icon_file);
$status_icon4->set_tooltip_text("Set From File Example");
$status_icon4->set_visible(true);

echo "All status icons created successfully without warnings!\n";
echo "Note: On some systems, you may need a system tray to see the icons.\n";

// Keep the script running to see the icons
// In a real application, you would typically have a GTK main loop
// Uncomment the following lines if you want to see the icons:
// Gtk::main();

// Clean up
unlink($icon_file);

echo "Example completed.\n";
