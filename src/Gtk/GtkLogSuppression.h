
#ifndef _PHPGTK_GTKLOGSUPPRESSION_H_
#define _PHPGTK_GTKLOGSUPPRESSION_H_

#include <gtk/gtk.h>

/**
 * Custom log handler to suppress GTK 3 bugs and warnings
 *
 * This handler suppresses known GTK 3 issues:
 * 1. gtk_widget_get_scale_factor: When loading icons or performing certain operations,
 *    GTK internally calls gtk_widget_get_scale_factor on non-widget objects (like GtkStatusIcon),
 *    causing a critical warning.
 * 2. size_allocate loop: GtkScrolledWindow or child widgets may call gtk_widget_queue_resize()
 *    during size_allocate, creating a resize loop. This is a known GTK 3 issue that occurs
 *    in certain layout scenarios and is generally harmless.
 *
 * These warnings can break applications in environments where warnings are configured as fatal.
 */
inline void suppress_gtk_warnings(const gchar *log_domain, GLogLevelFlags log_level,
                                  const gchar *message, gpointer user_data) {
  (void)log_domain;
  (void)log_level;
  (void)user_data;

  // Suppress the specific gtk_widget_get_scale_factor warning
  // Expected message format: "gtk_widget_get_scale_factor: assertion 'GTK_IS_WIDGET (widget)'
  // failed" String matching is necessary as GTK doesn't provide error codes for log messages
  if (g_strstr_len(message, -1, "gtk_widget_get_scale_factor") &&
      g_strstr_len(message, -1, "GTK_IS_WIDGET")) {
    // Silently ignore this specific warning
    return;
  }

  // Suppress the size_allocate loop warning
  // Expected message format: "GtkScrolledWindow 0x... or a child called gtk_widget_queue_resize()
  // during size_allocate()." This occurs when widgets trigger resize during size allocation,
  // creating a loop
  if (g_strstr_len(message, -1, "gtk_widget_queue_resize()") &&
      g_strstr_len(message, -1, "size_allocate")) {
    // Silently ignore this specific warning
    return;
  }

  // For all other messages, use default handler
  g_log_default_handler(log_domain, log_level, message, user_data);
}

/**
 * RAII wrapper for GTK log suppression
 *
 * Automatically installs log handler on construction and removes it on destruction.
 * This ensures proper cleanup even when exceptions are thrown.
 *
 * Usage:
 *   {
 *       GtkLogSuppressor suppressor;
 *       // Call GTK functions that might trigger the warnings
 *       gtk_status_icon_set_from_pixbuf(...);
 *   } // Automatic cleanup when suppressor goes out of scope
 */
class GtkLogSuppressor {
 private:
  GLogLevelFlags old_fatal_mask;
  guint critical_handler_id;
  guint warning_handler_id;

 public:
  GtkLogSuppressor() {
    // Disable fatal behavior and install custom log handlers
    old_fatal_mask = g_log_set_always_fatal((GLogLevelFlags)0);

    // Handle both critical and warning levels
    critical_handler_id =
        g_log_set_handler("Gtk", G_LOG_LEVEL_CRITICAL, suppress_gtk_warnings, NULL);
    warning_handler_id =
        g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING, suppress_gtk_warnings, NULL);
  }

  ~GtkLogSuppressor() {
    // Restore original state
    g_log_remove_handler("Gtk", critical_handler_id);
    g_log_remove_handler("Gtk", warning_handler_id);
    g_log_set_always_fatal(old_fatal_mask);
  }

  // Prevent copying and moving
  GtkLogSuppressor(const GtkLogSuppressor &) = delete;
  GtkLogSuppressor &operator=(const GtkLogSuppressor &) = delete;
  GtkLogSuppressor(GtkLogSuppressor &&) = delete;
  GtkLogSuppressor &operator=(GtkLogSuppressor &&) = delete;
};

#endif
