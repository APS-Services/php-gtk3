
#ifndef _PHPGTK_GTK_H_
#define _PHPGTK_GTK_H_

#include <phpcpp.h>
#include <iostream>
#include <gtk/gtk.h>

/**
 * Dependency:
 *  show_uri_on_window
 */
#include "GtkWindow.h"

/**
 *
 */
class Gtk_ : public Php::Base {
  /**
   * Privates
   */
 private:
  /**
   *
   */
  GtkWidget *widget{};

  struct st_timeout_add;

  /**
   * Publics
   */
 public:
  /**
   *  C++ constructor/destructor
   */
  Gtk_();

  /**
   * Gtk loop
   *		https://developer.gnome.org/gtk3/stable/gtk3-General.html#gtk-main
   */
  static void main();

  /**
   * Gtk main quit
   *		https://developer.gnome.org/gtk3/stable/gtk3-General.html#gtk-main-quit
   */
  static void main_quit();

  static Php::Value timeout_add(Php::Parameters &parameters);
  static Php::Value source_remove(Php::Parameters &parameters);
  static Php::Value is_destroyed(Php::Parameters &parameters);
  static Php::Value show_uri_on_window(Php::Parameters &parameters);
  static gint timeout_add_callback(gpointer data);

  /**
   * Install a handler that is called when a PHP signal/callback handler throws.
   *
   * Such a throwable cannot be allowed to propagate (it would unwind across
   * GLib's C frames), so it is reported here instead of reaching a PHP
   * try/catch around Gtk::main().
   *
   *   Gtk::set_exception_handler(
   *       function (string $message, string $origin, int $code) { ... });
   *
   * $origin is the signal name for signal handlers; for other callbacks it is
   * the installing method (e.g. "Gtk::timeout_add").
   *
   * Pass null to remove the handler. Without one, failures are reported with
   * g_critical() on stderr.
   *
   * Only the message and code survive - PHP-CPP does not expose the original
   * Throwable object, so class, file, line and trace are not available.
   */
  static void set_exception_handler(Php::Parameters &parameters);

  static Php::Value events_pending();
  static Php::Value main_do_event(Php::Parameters &parameters);
  static Php::Value main_iteration();
  static Php::Value get_major_version();
  static Php::Value get_micro_version();
  static Php::Value get_minor_version();
  static void init();
};

#endif