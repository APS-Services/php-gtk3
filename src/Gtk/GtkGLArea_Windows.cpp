// Windows implementation of GtkGLArea
// This file is included in GtkGLArea.cpp when building on Windows
// Uses libepoxy for OpenGL dispatch (provided by GTK3 on Windows)

#ifdef _WIN32

#include "GtkGLArea.h"
#include <windows.h>

// On Windows, GTK3 is built with OpenGL support via epoxy
// We include the epoxy headers that come with the GTK3 installation
#include <epoxy/gl.h>

GtkGLArea_::GtkGLArea_() = default;
GtkGLArea_::~GtkGLArea_() = default;

/**
 * Create a new GtkGLArea widget
 */
void GtkGLArea_::__construct() {
  instance = (gpointer *)gtk_gl_area_new();
}

/**
 * Ensures that the GdkGLContext used by the widget is associated with the widget.
 * This makes the GL context current and allows GL calls.
 */
void GtkGLArea_::make_current() {
  gtk_gl_area_make_current(GTK_GL_AREA(instance));
}

/**
 * Marks that the currently rendered frame needs to be redrawn
 */
void GtkGLArea_::queue_render() {
  gtk_gl_area_queue_render(GTK_GL_AREA(instance));
}

/**
 * Ensures that the widget's framebuffer object is complete
 */
void GtkGLArea_::attach_buffers() {
  gtk_gl_area_attach_buffers(GTK_GL_AREA(instance));
}

/**
 * Retrieves the GdkGLContext used by the area
 * Returns a Php::Value wrapping the context (as integer pointer for now)
 */
Php::Value GtkGLArea_::get_context() {
  GdkGLContext *context = gtk_gl_area_get_context(GTK_GL_AREA(instance));
  // Return the pointer as a numeric value
  // Note: Full GdkGLContext wrapper class could be implemented if needed
  return Php::Value((int64_t)context);
}

/**
 * Sets whether the GtkGLArea should use an alpha channel
 */
void GtkGLArea_::set_has_alpha(Php::Parameters &parameters) {
  gboolean has_alpha = (gboolean)parameters[0];
  gtk_gl_area_set_has_alpha(GTK_GL_AREA(instance), has_alpha);
}

/**
 * Returns whether the area has an alpha channel
 */
Php::Value GtkGLArea_::get_has_alpha() {
  return gtk_gl_area_get_has_alpha(GTK_GL_AREA(instance));
}

/**
 * Sets whether the GtkGLArea should use a depth buffer
 */
void GtkGLArea_::set_has_depth_buffer(Php::Parameters &parameters) {
  gboolean has_depth_buffer = (gboolean)parameters[0];
  gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(instance), has_depth_buffer);
}

/**
 * Returns whether the area has a depth buffer
 */
Php::Value GtkGLArea_::get_has_depth_buffer() {
  return gtk_gl_area_get_has_depth_buffer(GTK_GL_AREA(instance));
}

/**
 * Sets whether the GtkGLArea should use a stencil buffer
 */
void GtkGLArea_::set_has_stencil_buffer(Php::Parameters &parameters) {
  gboolean has_stencil_buffer = (gboolean)parameters[0];
  gtk_gl_area_set_has_stencil_buffer(GTK_GL_AREA(instance), has_stencil_buffer);
}

/**
 * Returns whether the area has a stencil buffer
 */
Php::Value GtkGLArea_::get_has_stencil_buffer() {
  return gtk_gl_area_get_has_stencil_buffer(GTK_GL_AREA(instance));
}

/**
 * Sets whether the GtkGLArea is in auto render mode
 */
void GtkGLArea_::set_auto_render(Php::Parameters &parameters) {
  gboolean auto_render = (gboolean)parameters[0];
  gtk_gl_area_set_auto_render(GTK_GL_AREA(instance), auto_render);
}

/**
 * Returns whether the area is in auto render mode
 */
Php::Value GtkGLArea_::get_auto_render() {
  return gtk_gl_area_get_auto_render(GTK_GL_AREA(instance));
}

/**
 * Sets the required OpenGL version
 * Parameters: major (int), minor (int)
 */
void GtkGLArea_::set_required_version(Php::Parameters &parameters) {
  int major = parameters[0];
  int minor = parameters[1];
  gtk_gl_area_set_required_version(GTK_GL_AREA(instance), major, minor);
}

/**
 * Gets the required OpenGL version
 * Parameters: &major (reference), &minor (reference)
 */
void GtkGLArea_::get_required_version(Php::Parameters &parameters) {
  int major, minor;
  gtk_gl_area_get_required_version(GTK_GL_AREA(instance), &major, &minor);

  // Set the values in the reference parameters
  parameters[0] = major;
  parameters[1] = minor;
}

/**
 * Gets the current error set on the area, if any
 */
Php::Value GtkGLArea_::get_error() {
  GError *error = gtk_gl_area_get_error(GTK_GL_AREA(instance));
  if (error) {
    std::string error_message(error->message);
    return error_message;
  }
  return nullptr;
}

#endif // _WIN32
