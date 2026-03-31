#include <gtk/gtk.h>
#include <phpcpp.h>

#include <iostream>

/**
 * @todo Add all properties
 *
 * https://developer.gnome.org/gdk3/stable/gdk3-Event-Structures.html
 */
class GdkEventAny_ : public Php::Base {
 public:
  /**
   *  c++ constructor
   */
  GdkEventAny_() = default;

  /**
   *  c++ destructor
   */
  ~GdkEventAny_() override = default;
};