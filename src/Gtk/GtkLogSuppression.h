
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
 * Log writer that suppresses the GDK noise of a session without a desktop
 *
 * Two messages, one cause: a Windows session that has no real desktop - a remote
 * desktop session, one without a running dwm.exe, a process started by a service
 * or a scheduled task. A GitHub Actions runner installed as a service is one, and
 * it is where both of these were found.
 *
 * "DwmEnableBlurBehindWindow (...) failed: 80263001", once per native window: from
 * Windows 8 on GDK takes desktop composition for granted (gdkscreen-win32.c sets
 * always_composited for 6.2+, so gdk_screen_is_composited() never asks
 * DwmIsCompositionEnabled()) and calls DwmEnableBlurBehindWindow() at the end of
 * _gdk_window_impl_new(). Where composition is off the call returns
 * DWM_E_COMPOSITIONDISABLED, and GDK warns although it ignores the result: the
 * window only misses blur-behind transparency, which an uncomposited desktop would
 * not have shown anyway.
 *
 * "gdk_monitor_get_workarea: assertion 'GDK_IS_MONITOR (monitor)' failed", and the
 * same for get_geometry: GTK looks a monitor up and uses it without a NULL check -
 * gtkwindow.c (every WIN_POS_CENTER window), gtkmenu.c, gtkcombobox.c,
 * gtktreeview.c and four more call sites in 3.24. With no monitors in the session
 * the lookup returns NULL, GDK refuses the call and GTK positions the window from
 * an uninitialised rectangle. Dropping this one hides a real consequence, so it is
 * a deliberate choice: the placement is undefined either way, no window is visible
 * in such a session, and hundreds of lines per run are worse than the signal is
 * worth. Only this assertion is dropped; every other Gdk critical still shows.
 *
 * Both have to be filtered here rather than in a g_log_set_handler() handler. GTK
 * is built with -DG_LOG_USE_STRUCTURED=1 (its meson.build), so a g_warning() in GDK
 * expands to g_log_structured_standard() and goes straight to the writer without
 * consulting a legacy per-domain handler. The assertions do come through g_log(),
 * from g_return_if_fail_warning(), but with no handler registered for them they
 * reach the writer as well, so one writer covers both.
 */
inline GLogWriterOutput suppress_gdk_desktopless_noise_writer(GLogLevelFlags log_level,
                                                             const GLogField *fields,
                                                             gsize n_fields,
                                                             gpointer user_data) {
  const gchar *log_domain = nullptr;
  const gchar *message = nullptr;
  gssize message_length = -1;

  for (gsize i = 0; i < n_fields; i++) {
    if (g_strcmp0(fields[i].key, "GLIB_DOMAIN") == 0) {
      log_domain = (const gchar *)fields[i].value;
    } else if (g_strcmp0(fields[i].key, "MESSAGE") == 0) {
      message = (const gchar *)fields[i].value;
      // Negative for a nul terminated string, which is what g_strstr_len() wants too
      message_length = fields[i].length;
    }
  }

  // String matching is necessary as GDK doesn't provide error codes for log messages
  if (g_strcmp0(log_domain, "Gdk") == 0 && message != nullptr) {
    // "gdk/win32/gdkwindow-win32.c:527: DwmEnableBlurBehindWindow (<hwnd>) failed: 80263001"
    if ((log_level & G_LOG_LEVEL_WARNING) != 0 &&
        g_strstr_len(message, message_length, "DwmEnableBlurBehindWindow") != nullptr) {
      return G_LOG_WRITER_HANDLED;
    }

    // "gdk_monitor_get_workarea: assertion 'GDK_IS_MONITOR (monitor)' failed"
    if ((log_level & G_LOG_LEVEL_CRITICAL) != 0 &&
        g_strstr_len(message, message_length, "GDK_IS_MONITOR") != nullptr) {
      return G_LOG_WRITER_HANDLED;
    }
  }

  // Everything else is written the way GLib would have written it anyway
  return g_log_writer_default(log_level, fields, n_fields, user_data);
}

/**
 * Install the GDK log writer for the lifetime of the process
 *
 * Called once from Gtk::init(), before gtk_init(): unlike the scale factor warning
 * there is no call of ours to wrap, GDK emits these whenever it creates or places a
 * window. Only the Windows build installs it; both messages come from GDK's Win32
 * backend, or from a monitor list only Windows leaves empty.
 *
 * GLib allows one writer per process and makes a second g_log_set_writer_func() a
 * g_error(), which aborts - so this must stay the only call in the process, and a
 * script must not install a writer of its own.
 */
inline void install_gdk_log_suppression() {
#ifdef G_OS_WIN32
  static bool installed = false;

  if (installed) {
    return;
  }

  installed = true;
  g_log_set_writer_func(suppress_gdk_desktopless_noise_writer, nullptr, nullptr);
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
