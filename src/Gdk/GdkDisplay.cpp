
#include "GdkDisplay.h"

/**
 * Constructor
 */
GdkDisplay_::GdkDisplay_() = default;

/**
 * Destructor
 */
GdkDisplay_::~GdkDisplay_() = default;

/**
 * Return original GtkWidget
 */
GdkDisplay *GdkDisplay_::get_instance()
{
    return instance;
}

/**
 * Set the original GdkDisplay
 */
void GdkDisplay_::set_instance(GdkDisplay *screen)
{
    instance = screen;
}

/**
 * https://developer.gnome.org/gdk3/stable/GdkDisplay.html#gdk-display-get-default
 */
Php::Value GdkDisplay_::get_default()
{
    GdkDisplay *returndedValue = gdk_display_get_default();

    // Validate that we got a valid display connection
    if (returndedValue == NULL) {
        throw Php::Exception("Failed to get default display. Make sure GTK is initialized and a graphical display backend is available.");
    }

    GdkDisplay_ *returnValue = new GdkDisplay_();
    returnValue->set_instance(returndedValue);

    return Php::Object("GdkDisplay", returnValue);
}

/**
 * https://developer.gnome.org/gdk3/stable/GdkDisplay.html#gdk-display-get-primary-monitor
 */
Php::Value GdkDisplay_::get_primary_monitor()
{
    if (instance == NULL) {
        throw Php::Exception("GdkDisplay instance is NULL. Cannot get primary monitor.");
    }

    GdkMonitor *returndedValue = gdk_display_get_primary_monitor(GDK_DISPLAY(instance));

    GdkMonitor_ *returnValue = new GdkMonitor_();
    returnValue->set_instance(returndedValue);

    return Php::Object("GdkMonitor", returnValue);
}

Php::Value GdkDisplay_::get_default_screen()
{
    if (instance == NULL) {
        throw Php::Exception("GdkDisplay instance is NULL. Cannot get default screen.");
    }

    GdkScreen* ret = gdk_display_get_default_screen(GDK_DISPLAY(instance));

    if (ret == NULL) {
        throw Php::Exception("Failed to get default screen from display.");
    }

    GdkScreen_ *returnValue = new GdkScreen_();
    returnValue->set_instance(ret);

    return Php::Object("GdkScreen", returnValue);
}

/**
 * https://developer.gnome.org/gdk3/stable/GdkDisplay.html#gdk-display-flush
 * Flushes any requests queued for the windowing system; this happens automatically
 * when the main loop blocks waiting for new events, but if your application
 * is drawing without returning control to the main loop, you may need to call this
 * function explicitly. A common case where this function needs to be called is when
 * an application is executing drawing commands from a thread other than the thread
 * where the main loop is running.
 */
void GdkDisplay_::flush()
{
    if (instance == NULL) {
        throw Php::Exception("GdkDisplay instance is NULL. Cannot flush display.");
    }

    gdk_display_flush(GDK_DISPLAY(instance));
}

/**
 * https://developer.gnome.org/gdk3/stable/GdkDisplay.html#gdk-display-sync
 * Flushes any requests queued for the windowing system and waits until all
 * requests have been handled. This is rarely needed by applications. It's
 * mainly used for debugging and testing purposes.
 */
void GdkDisplay_::sync()
{
    if (instance == NULL) {
        throw Php::Exception("GdkDisplay instance is NULL. Cannot sync display.");
    }

    gdk_display_sync(GDK_DISPLAY(instance));
}