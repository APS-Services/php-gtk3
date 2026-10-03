
#include "GdkMonitor.h"

/**
 * Constructor
 */
GdkMonitor_::GdkMonitor_() = default;

/**
 * Destructor
 */
GdkMonitor_::~GdkMonitor_() = default;

/**
 * Whether the wrapped pointer is still a monitor
 *
 * The instance can be NULL - gdk_display_get_primary_monitor() returns NULL on
 * a session that enumerated no monitors - and GDK drops the GdkMonitor when a
 * monitor is unplugged. Passing either to GDK only prints a GDK_IS_MONITOR
 * assertion and leaves the out parameter untouched, which would reach PHP as
 * uninitialised stack values, so the getters below check first and return null.
 */
static bool monitor_is_valid(GdkMonitor *monitor) {
  return monitor != nullptr && GDK_IS_MONITOR(monitor);
}

/**
 * Return original GtkWidget
 */
GdkMonitor *GdkMonitor_::get_instance() {
  return instance;
}

/**
 * Set the original GdkMonitor
 */
void GdkMonitor_::set_instance(GdkMonitor *monitor) {
  instance = monitor;
}

/**
 * https://developer.gnome.org/gdk3/stable/GdkMonitor.html#gdk-monitor-get-width-mm
 */
Php::Value GdkMonitor_::get_width_mm() {
  if (!monitor_is_valid(instance)) {
    return nullptr;
  }

  return gdk_monitor_get_width_mm(GDK_MONITOR(instance));
}

/**
 * https://developer.gnome.org/gdk3/stable/GdkMonitor.html#gdk-monitor-get-height-mm
 */
Php::Value GdkMonitor_::get_height_mm() {
  if (!monitor_is_valid(instance)) {
    return nullptr;
  }

  return gdk_monitor_get_height_mm(GDK_MONITOR(instance));
}

/**
 * https://developer.gnome.org/gdk3/stable/GdkMonitor.html#gdk-monitor-get-workarea
 */
Php::Value GdkMonitor_::get_workarea() {
  if (!monitor_is_valid(instance)) {
    return nullptr;
  }

  // Allocate a GdkRectangle on the stack
  GdkRectangle workarea = {0, 0, 0, 0};

  // Fill in the workarea details
  gdk_monitor_get_workarea(GDK_MONITOR(instance), &workarea);

  // Create the return array
  Php::Value arr;
  arr["x"] = workarea.x;
  arr["y"] = workarea.y;
  arr["width"] = workarea.width;
  arr["height"] = workarea.height;

  return arr;
}

Php::Value GdkMonitor_::get_geometry() {
  if (!monitor_is_valid(instance)) {
    return nullptr;
  }

  GdkRectangle rect = {0, 0, 0, 0};
  gdk_monitor_get_geometry((GdkMonitor *)instance, &rect);

  Php::Value arr;
  arr["x"] = rect.x;
  arr["y"] = rect.y;
  arr["width"] = rect.width;
  arr["height"] = rect.height;

  return arr;
}