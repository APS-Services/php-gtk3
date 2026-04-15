
#ifndef _PHPGTK_GTKGLAREA_H_
#define _PHPGTK_GTKGLAREA_H_

#include <phpcpp.h>
#include <gtk/gtk.h>

#ifndef _WIN32
// Unix/Linux: epoxy headers (OpenGL dispatch library)
#include <epoxy/gl.h>
#endif

#include "GtkWidget.h"

/**
 * GtkGLArea_
 *
 * Cross-platform OpenGL widget:
 * - Unix/Linux: Uses GtkGLArea with libepoxy
 * - Windows: Uses GtkGLArea with libepoxy (via GTK3 build)
 *
 * https://developer.gnome.org/gtk3/stable/GtkGLArea.html
 */
class GtkGLArea_ : public GtkWidget_ {
  /**
   * Publics
   */
 public:
  /**
   *  C++ constructor and destructor
   */
  GtkGLArea_();
  ~GtkGLArea_();

  void __construct();

  void make_current();

  void queue_render();

  void attach_buffers();

  Php::Value get_context();

  void set_has_alpha(Php::Parameters &parameters);

  Php::Value get_has_alpha();

  void set_has_depth_buffer(Php::Parameters &parameters);

  Php::Value get_has_depth_buffer();

  void set_has_stencil_buffer(Php::Parameters &parameters);

  Php::Value get_has_stencil_buffer();

  void set_auto_render(Php::Parameters &parameters);

  Php::Value get_auto_render();

  void set_required_version(Php::Parameters &parameters);

  void get_required_version(Php::Parameters &parameters);

  Php::Value get_error();

  /**
   * Private members
   */
 private:
#ifndef _WIN32
  // Unix/Linux: No additional members needed
#else
  // Windows: No additional members needed (uses same GTK3 infrastructure)
#endif
};

#endif
