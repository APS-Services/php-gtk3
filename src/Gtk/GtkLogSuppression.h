
#ifndef _PHPGTK_GTKLOGSUPPRESSION_H_
#define _PHPGTK_GTKLOGSUPPRESSION_H_

#include <gtk/gtk.h>

/**
 * Custom log handler to suppress GTK 3 bug with gtk_widget_get_scale_factor
 *
 * When loading icons or performing certain operations, GTK internally calls
 * gtk_widget_get_scale_factor on non-widget objects (like GtkStatusIcon),
 * causing a critical warning. This is a known GTK 3 issue and the warning
 * is harmless but can break applications in environments where critical
 * warnings are configured as fatal.
 */
inline void suppress_scale_factor_warning(const gchar *log_domain, GLogLevelFlags log_level,
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

  // For all other messages, use default handler
  g_log_default_handler(log_domain, log_level, message, user_data);
}

/**
 * Custom log handler to suppress GDK's Win32 blur-behind warning
 *
 * From Windows 8 on, GDK takes desktop composition for granted
 * (gdkscreen-win32.c sets always_composited for 6.2+, so gdk_screen_is_composited()
 * never asks DwmIsCompositionEnabled()) and calls DwmEnableBlurBehindWindow() for
 * every native window it creates, at the end of _gdk_window_impl_new(). Where
 * composition really is off - a remote desktop session, a session without a running
 * dwm.exe, a process carrying the DISABLEDWM compatibility layer - that call returns
 * DWM_E_COMPOSITIONDISABLED (0x80263001) and GDK warns, once per window, dialog, menu
 * and tooltip. GDK ignores the result: the window only misses blur-behind
 * transparency, which it would not have got on an uncomposited desktop anyway, so
 * nothing but the log noise is left to deal with.
 */
inline void suppress_dwm_blur_behind_warning(const gchar *log_domain, GLogLevelFlags log_level,
                                             const gchar *message, gpointer user_data) {
  // Expected message format:
  // "gdk/win32/gdkwindow-win32.c:527: DwmEnableBlurBehindWindow (<hwnd>) failed: 80263001"
  // String matching is necessary as GDK doesn't provide error codes for log messages
  if (g_strstr_len(message, -1, "DwmEnableBlurBehindWindow")) {
    // Silently ignore this specific warning
    return;
  }

  // For all other messages, use default handler
  g_log_default_handler(log_domain, log_level, message, user_data);
}

/**
 * Install the GDK log handler for the lifetime of the process
 *
 * Called once from Gtk::init(), before gtk_init(): unlike the scale factor warning
 * there is no call of ours to wrap, GDK emits this one whenever it creates a window.
 * Only the Windows build installs it; the warning comes from GDK's Win32 backend.
 */
inline void install_gdk_log_suppression() {
#ifdef G_OS_WIN32
  static bool installed = false;

  if (installed) {
    return;
  }

  installed = true;
  g_log_set_handler("Gdk", G_LOG_LEVEL_WARNING, suppress_dwm_blur_behind_warning, NULL);
#endif
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
    handler_id =
        g_log_set_handler("Gtk", G_LOG_LEVEL_CRITICAL, suppress_scale_factor_warning, NULL);
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
