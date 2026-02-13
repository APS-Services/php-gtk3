<?php
/**
 * GtkStatusIcon Example
 * 
 * This example demonstrates how to create and use a system tray icon (status icon).
 * 
 * IMPORTANT NOTES:
 * - GtkStatusIcon is DEPRECATED and only works on X11
 * - On Wayland, the icon may appear but events (clicks) will NOT work properly
 * - If running on Wayland, use: GDK_BACKEND=x11 php thisfile.php
 * - For production Wayland apps, consider using AppIndicator instead
 * 
 * Expected signals for GtkStatusIcon:
 * - activate: Left-click on the icon
 * - popup-menu: Right-click on the icon  
 * - button-press-event: Any button press (with event details)
 * - button-release-event: Any button release (with event details)
 * - scroll-event: Mouse wheel scroll
 * - size-changed: Icon size changed
 */

// Check if we're on Wayland and warn the user
$display = getenv('WAYLAND_DISPLAY');
if ($display !== false && $display !== '') {
    echo "WARNING: Wayland detected ($display)\n";
    echo "GtkStatusIcon does not work properly on Wayland.\n";
    echo "Events (clicks) will not be received.\n";
    echo "To run this example, use: GDK_BACKEND=x11 php " . basename(__FILE__) . "\n";
    echo "\nContinuing anyway for demonstration...\n\n";
}

// Create a simple window (just to keep the app running)
$window = new GtkWindow();
$window->set_title("StatusIcon Example");
$window->set_default_size(300, 200);
$window->connect("destroy", function() {
    Gtk::main_quit();
});

$label = new GtkLabel("This app has a system tray icon.\nCheck your system tray!");
$window->add($label);
$window->show_all();

// Create the status icon
$icon = new GtkStatusIcon();

// Set icon from icon name (use a standard icon)
$icon->set_from_icon_name("applications-system");

// Alternative ways to set the icon:
// $icon->set_from_file("/path/to/icon.png");
// $icon->set_from_stock("gtk-about");

// Set tooltip
$icon->set_tooltip_text("Click me! (Only works on X11)");

// Set title
$icon->set_title("My Application");

// Make it visible
$icon->set_visible(true);

// Connect to the 'activate' signal (left-click)
// NOTE: This will ONLY work on X11, not on Wayland
$icon->connect("activate", function() {
    echo "Icon was left-clicked (activate signal)\n";
    
    $dialog = new GtkMessageDialog(
        null,
        GtkDialogFlags::MODAL,
        GtkMessageType::INFO,
        GtkButtonsType::OK,
        "Status icon was activated!\n\nThis only works on X11."
    );
    $dialog->run();
    $dialog->destroy();
});

// Connect to the 'popup-menu' signal (right-click)
// NOTE: This will ONLY work on X11, not on Wayland
$icon->connect("popup-menu", function($icon, $button, $activate_time) {
    echo "Icon was right-clicked (popup-menu signal)\n";
    echo "Button: $button, Time: $activate_time\n";
    
    // Create a popup menu
    $menu = new GtkMenu();
    
    $item1 = new GtkMenuItem("Show Window");
    $item1->connect("activate", function() use ($window) {
        $window->present();
    });
    $menu->append($item1);
    
    $item2 = new GtkMenuItem("About");
    $item2->connect("activate", function() {
        $dialog = new GtkMessageDialog(
            null,
            GtkDialogFlags::MODAL,
            GtkMessageType::INFO,
            GtkButtonsType::OK,
            "StatusIcon Example\n\nDemonstrates system tray functionality.\n\nNOTE: Only works on X11!"
        );
        $dialog->run();
        $dialog->destroy();
    });
    $menu->append($item2);
    
    $separator = new GtkSeparatorMenuItem();
    $menu->append($separator);
    
    $item3 = new GtkMenuItem("Quit");
    $item3->connect("activate", function() {
        Gtk::main_quit();
    });
    $menu->append($item3);
    
    $menu->show_all();
    
    // Position and show the menu at the icon location
    $menu->popup(
        null,                    // parent menu shell
        null,                    // parent menu item
        array($icon, "position_menu"), // position function
        null,                    // data
        $button,                 // button
        $activate_time           // activate time
    );
});

// Optional: Connect to button-press-event for more control
// This gives access to the raw GdkEventButton
$icon->connect("button-press-event", function($icon, $event) {
    echo "Button press event received\n";
    
    // You can check which button was pressed
    $button = $event->button;
    echo "Button number: $button\n";
    
    // Return false to allow other handlers to process
    return false;
});

// Optional: Connect to scroll-event
$icon->connect("scroll-event", function($icon, $event) {
    echo "Scroll event received\n";
    $direction = $event->direction;
    echo "Direction: $direction\n";
    return false;
});

// Check if the icon is embedded in the system tray
if ($icon->is_embedded()) {
    echo "Status icon is successfully embedded in the system tray\n";
} else {
    echo "WARNING: Status icon is NOT embedded in the system tray\n";
    echo "This usually means:\n";
    echo "1. No system tray is available (common on Wayland)\n";
    echo "2. The desktop environment doesn't support XEmbed tray icons\n";
}

echo "\nApplication started. The status icon should appear in your system tray.\n";
echo "Try clicking on it (left-click and right-click).\n";
echo "Close this window or use the tray menu to quit.\n\n";

Gtk::main();
