
#ifndef _PHPGTK_GTKLOGSUPPRESSION_H_
#define _PHPGTK_GTKLOGSUPPRESSION_H_

#include <gtk/gtk.h>

/**
 * Custom log handler to suppress known-noisy GTK 3 CRITICAL warnings
 *
 * Currently suppresses the following harmless-but-noisy warnings emitted from
 * GTK 3 internals. String matching is required because GTK does not provide
 * error codes for log messages.
 *
 *  1. gtk_widget_get_scale_factor: assertion 'GTK_IS_WIDGET (widget)' failed
 *     When loading icons or performing certain operations, GTK internally
 *     calls gtk_widget_get_scale_factor on non-widget objects (like
 *     GtkStatusIcon), causing a critical warning.
 *
 *  2. gtk_distribute_natural_allocation: assertion 'extra_space >= 0' failed
 *     A well-known upstream GTK 3 bug (see GNOME/gtk#1735 and many downstream
 *     reports) triggered from GTK's own size-negotiation code when a container
 *     is transiently allocated less space than the sum of its children's
 *     minimum sizes during a layout pass. The warning is harmless -- GTK
 *     handles the case gracefully -- but noisy and can break applications
 *     that treat CRITICAL warnings as fatal (G_DEBUG=fatal-criticals).
 *
 * Both messages are harmless in practice but can break applications in
 * environments where critical warnings are configured as fatal.
 */
inline void suppress_known_gtk_warnings(const gchar *log_domain, GLogLevelFlags log_level,
                                        const gchar *message, gpointer user_data) {
  (void)log_domain;
  (void)log_level;
  (void)user_data;

  if (message != NULL) {
    // Suppress: gtk_widget_get_scale_factor: assertion 'GTK_IS_WIDGET (widget)' failed
    if (g_strstr_len(message, -1, "gtk_widget_get_scale_factor") &&
        g_strstr_len(message, -1, "GTK_IS_WIDGET")) {
      return;
    }

    // Suppress: gtk_distribute_natural_allocation: assertion 'extra_space >= 0' failed
    if (g_strstr_len(message, -1, "gtk_distribute_natural_allocation") &&
        g_strstr_len(message, -1, "extra_space")) {
      return;
    }
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
 *       // Call GTK functions that might trigger the warning
 *       gtk_status_icon_set_from_pixbuf(...);
 *   } // Automatic cleanup when suppressor goes out of scope
 */
class GtkLogSuppressor {
 private:
  GLogLevelFlags old_fatal_mask;
  guint handler_id;

 public:
  GtkLogSuppressor() {
    // Disable fatal behavior and install custom log handler
    old_fatal_mask = g_log_set_always_fatal((GLogLevelFlags)0);
    handler_id = g_log_set_handler("Gtk", G_LOG_LEVEL_CRITICAL, suppress_known_gtk_warnings, NULL);
  }

  ~GtkLogSuppressor() {
    // Restore original state
    g_log_remove_handler("Gtk", handler_id);
    g_log_set_always_fatal(old_fatal_mask);
  }

  // Prevent copying and moving
  GtkLogSuppressor(const GtkLogSuppressor &) = delete;
  GtkLogSuppressor &operator=(const GtkLogSuppressor &) = delete;
  GtkLogSuppressor(GtkLogSuppressor &&) = delete;
  GtkLogSuppressor &operator=(GtkLogSuppressor &&) = delete;
};

#endif
