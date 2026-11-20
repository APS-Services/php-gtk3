
#include "GtkTextTag.h"

/**
 * Constructor
 */
GtkTextTag_::GtkTextTag_() = default;

/**
 * Destructor
 */
GtkTextTag_::~GtkTextTag_() = default;

/**
 * Return original GtkTreeIter
 */
GtkTextTag *GtkTextTag_::get_tag() {
  return GTK_TEXT_TAG(instance);
}

/**
 * Set the original GtkTextTag
 */
void GtkTextTag_::set_tag(GtkTextTag *passed_tag) {
  instance = (gpointer *)passed_tag;
}

void GtkTextTag_::__construct(Php::Parameters &parameters) {
  // no name (or null): an anonymous tag
  std::string s_name;
  const gchar *name = nullptr;
  if (!parameters.empty() && !parameters[0].isNull()) {
    s_name = parameters[0].stringValue();
    name = s_name.c_str();
  }

  instance = (gpointer *)gtk_text_tag_new(name);
}

Php::Value GtkTextTag_::get_priority() {
  gint ret = gtk_text_tag_get_priority(get_tag());

  return ret;
}

void GtkTextTag_::set_priority(Php::Parameters &parameters) {
  gint priority = (gint)parameters[0];

  gtk_text_tag_set_priority(get_tag(), priority);
}

Php::Value GtkTextTag_::event(Php::Parameters &parameters) {
  // GtkTextIter *iter;
  // if(parameters.size() > 2) {
  // 	Php::Value object_iter = parameters[2];
  // 	GtkTextIter_ *phpgtk_iter = (GtkTextIter_ *)object_iter.implementation();
  // 	iter = GTK_WIDGET(phpgtk_iter->get_instance());
  // }

  // GtkTextIter *iter;
  // if(parameters.size() > 2) {
  // 	Php::Value object_iter = parameters[2];
  // 	GtkTextIter_ *phpgtk_iter = (GtkTextIter_ *)object_iter.implementation();
  // 	iter = GTK_WIDGET(phpgtk_iter->get_instance());
  // }

  // gboolean ret = gtk_text_tag_event (tag, event_object, event, iter);

  // return ret;

  throw Php::Exception("GtkTextTag_::event not implemented");
  return -1;
}

void GtkTextTag_::tag_changed(Php::Parameters &parameters) {
  gboolean size_changed = (gboolean)parameters[0];

  gtk_text_tag_changed(get_tag(), size_changed);
}
