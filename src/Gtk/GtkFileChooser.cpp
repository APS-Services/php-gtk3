
#include "GtkFileChooser.h"

#include "../../php-gtk.h"

/**
 * Constructor
 */
GtkFileChooser_::GtkFileChooser_() = default;

/**
 * Destructor
 */
GtkFileChooser_::~GtkFileChooser_() = default;

Php::Value GtkFileChooser_::get_filename() {
  return phpgtk_take_string(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(instance)));
}
