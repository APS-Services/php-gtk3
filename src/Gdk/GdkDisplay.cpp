
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
GdkDisplay *GdkDisplay_::get_instance() {
  return instance;
}

/**
 * Set the original GdkDisplay
 */
void GdkDisplay_::set_instance(GdkDisplay *screen) {
  instance = screen;
}

/**
 * https://developer.gnome.org/gdk3/stable/GdkDisplay.html#gdk-display-get-default
 */
Php::Value GdkDisplay_::get_default() {
  GdkDisplay *returndedValue = gdk_display_get_default();

  GdkDisplay_ *returnValue = new GdkDisplay_();
  returnValue->set_instance(returndedValue);

  return Php::Object("GdkDisplay", returnValue);
}

/**
 * https://developer.gnome.org/gdk3/stable/GdkDisplay.html#gdk-display-get-primary-monitor
 *
 * The GDK function is documented (nullable): it returns NULL where the user
 * configured no primary monitor, and the Win32 backend returns it whenever the
 * session enumerated no monitors at all - a disconnected remote desktop
 * session, or a process started without an interactive desktop. Wrapping that
 * NULL would hand PHP a GdkMonitor whose every getter trips a GDK_IS_MONITOR
 * assertion, so null comes back instead, as in get_monitor() below.
 */
Php::Value GdkDisplay_::get_primary_monitor() {
  GdkMonitor *returndedValue = gdk_display_get_primary_monitor(GDK_DISPLAY(instance));

  if (returndedValue == nullptr) {
    return {};
  }

  GdkMonitor_ *returnValue = new GdkMonitor_();
  returnValue->set_instance(returndedValue);

  return Php::Object("GdkMonitor", returnValue);
}

Php::Value GdkDisplay_::get_default_screen() {
  GdkScreen *ret = gdk_display_get_default_screen(GDK_DISPLAY(instance));

  GdkScreen_ *returnValue = new GdkScreen_();
  returnValue->set_instance(ret);

  return Php::Object("GdkScreen", returnValue);
}

Php::Value GdkDisplay_::get_monitor(Php::Parameters &parameters) {
  if (parameters.empty()) {
    throw Php::Exception("parameter monitor_num is required");
  }

  // unpack monitor number
  int monitor_num = (int)parameters[0];

  // call gtk function
  GdkMonitor *ret = gdk_display_get_monitor(GDK_DISPLAY(instance), monitor_num);

  if (ret == nullptr) {
    return {};
  }

  // pack and return object
  GdkMonitor_ *return_parsed = new GdkMonitor_();
  return_parsed->set_instance(ret);
  return Php::Object("GdkMonitor", return_parsed);
}