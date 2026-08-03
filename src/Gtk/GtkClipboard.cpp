
#include "GtkClipboard.h"

#include "../../php-gtk.h"

/**
 * Struct for callback gpointer
 *
 * The user parameters are kept as a plain vector rather than a Php::Parameters:
 * Php::Parameters has no public default constructor, so a struct holding one
 * cannot be constructed normally - only the (invalid) malloc + memset trick
 * this struct used to be created with would compile.
 */
struct GtkClipboard_::st_request_callback {
  std::vector<Php::Value> user_parameters;
  Php::Object self_widget;
};

/**
 * Constructor
 */
GtkClipboard_::GtkClipboard_() = default;

/**
 * Destructor
 */
GtkClipboard_::~GtkClipboard_() = default;

void GtkClipboard_::__construct(Php::Parameters &parameters) {
  int a = parameters[0];
  instance = (gpointer *)gtk_clipboard_get(_GDK_MAKE_ATOM(a));
}

Php::Value GtkClipboard_::get_for_display(Php::Parameters &parameters) {
  // GdkDisplay *display;
  // if(parameters.size() > 0) {
  // 	Php::Value object_display = parameters[0];
  // 	GdkDisplay_ *phpgtk_display = (GdkDisplay_ *)object_display.implementation();
  // 	display = GTK_WIDGET(phpgtk_display->get_instance());
  // }

  // int int_selection = (int)parameters[1];
  // GdkSelection selection = (GdkSelection)int_selection;

  // GtkClipboard *ret = gtk_clipboard_get_for_display (GTK_CLIPBOARD(instance), display,
  // selection);

  // GtkClipboard_ *return_parsed = new GtkClipboard_();
  // return_parsed->set_instance((gpointer *)ret);
  // return Php::Object("GtkClipboard", return_parsed);

  throw Php::Exception("tkClipboard_::get_for_display not implemented");
  return -1;
}

Php::Value GtkClipboard_::get_display() {
  // GdkDisplay ret = gtk_clipboard_get_display (GTK_CLIPBOARD(instance));

  // return ret;

  throw Php::Exception("tkClipboard_::get_display not implemented");
  return -1;
}

Php::Value GtkClipboard_::set_with_data(Php::Parameters &parameters) {
  // GtkTargetEntry *targets;
  // if(parameters.size() > 0) {
  // 	Php::Value object_targets = parameters[0];
  // 	GtkTargetEntry_ *phpgtk_targets = (GtkTargetEntry_ *)object_targets.implementation();
  // 	targets = GTK_WIDGET(phpgtk_targets->get_instance());
  // }

  // guint n_targets = (guint)parameters[1];

  // gboolean ret = gtk_clipboard_set_with_data (GTK_CLIPBOARD(instance), targets, n_targets);

  // return ret;

  throw Php::Exception("tkClipboard_::set_with_data not implemented");
  return -1;
}

Php::Value GtkClipboard_::set_with_owner(Php::Parameters &parameters) {
  // GtkTargetEntry *targets;
  // if(parameters.size() > 0) {
  // 	Php::Value object_targets = parameters[0];
  // 	GtkTargetEntry_ *phpgtk_targets = (GtkTargetEntry_ *)object_targets.implementation();
  // 	targets = GTK_WIDGET(phpgtk_targets->get_instance());
  // }

  // guint n_targets = (guint)parameters[1];

  // gboolean ret = gtk_clipboard_set_with_owner (GTK_CLIPBOARD(instance), targets, n_targets);

  // return ret;

  throw Php::Exception("tkClipboard_::set_with_owner not implemented");
  return -1;
}

Php::Value GtkClipboard_::get_owner() {
  // GObject ret = gtk_clipboard_get_owner (GTK_CLIPBOARD(instance));

  // return ret;

  throw Php::Exception("tkClipboard_::get_owner not implemented");
  return -1;
}

void GtkClipboard_::clear() {
  gtk_clipboard_clear(GTK_CLIPBOARD(instance));
}

void GtkClipboard_::set_text(Php::Parameters &parameters) {
  std::string s_text = parameters[0];
  gchar *text = (gchar *)s_text.c_str();

  gint len = -1;
  if (parameters.size() >= 2) {
    len = (gint)parameters[1];
  }

  gtk_clipboard_set_text(GTK_CLIPBOARD(instance), text, len);
}

void GtkClipboard_::set_image(Php::Parameters &parameters) {
  GdkPixbuf *pixbuf;
  if (!parameters.empty()) {
    Php::Value object_pixbuf = parameters[0];
    GdkPixbuf_ *phpgtk_pixbuf = (GdkPixbuf_ *)object_pixbuf.implementation();
    pixbuf = phpgtk_pixbuf->get_instance();
  }

  gtk_clipboard_set_image(GTK_CLIPBOARD(instance), pixbuf);
}

void GtkClipboard_::request_contents(Php::Parameters &parameters) {
  // std::string s_user_function = parameters[0];
  // gchar *user_function = (gchar *)s_user_function.c_str();

  // gpointer user_data = (gpointer)parameters[1];

  // gtk_clipboard_request_contents (GTK_CLIPBOARD(instance), user_function, user_data);

  throw Php::Exception("tkClipboard_::request_contents not implemented");
}

void GtkClipboard_::request_text(Php::Parameters &parameters) {
  // Validate here, while we are still in PHP space and can throw. Inside the
  // callback a bad handler could only be reported, not raised.
  if (parameters.empty() || !parameters[0].isCallable()) {
    throw Php::Exception("GtkClipboard::request_text() expects a callable");
  }

  // Create user data param of callaback.
  //
  // Constructed, not malloc'd: the struct holds Php::Value members, so writing
  // into raw memory whose constructors never ran is undefined behaviour.
  // gtk_clipboard_request_text() invokes the callback exactly once, so
  // request_text_callback() takes ownership and deletes it.
  struct st_request_callback *callback_object = new struct st_request_callback();
  callback_object->user_parameters.assign(parameters.begin(), parameters.end());
  callback_object->self_widget = Php::Object("GtkClipboard", this);

  // handle the callback
  gtk_clipboard_request_text(GTK_CLIPBOARD(instance), request_text_callback,
                             (gpointer)callback_object);
}

void GtkClipboard_::request_text_callback(GtkClipboard *clipboard, const gchar *clipboard_text,
                                          gpointer user_data) {
  // Return to st_callback
  struct st_request_callback *callback_object = (struct st_request_callback *)user_data;

  // The handler, kept as a Php::Value rather than converted to std::string:
  // anything callable works that way - a closure, [$object, 'method'], an
  // invokable - matching every other callback in the binding. Reading it into a
  // std::string accepted only a plain function name, and turned a closure into
  // an "object of class Closure could not be converted to string" error.
  Php::Value callback_name = callback_object->user_parameters[0];

  // Create internal params, GtkClipboard + text + user_data...
  //
  // GTK passes NULL when the clipboard is empty or holds something that cannot
  // be converted to text, so the handler receives null in that case - assigning
  // the NULL straight into a Php::Value would crash.
  Php::Value internal_parameters;
  internal_parameters[0] = callback_object->self_widget;
  internal_parameters[1] =
      (clipboard_text != nullptr) ? Php::Value(clipboard_text) : Php::Value(nullptr);
  for (int i = 1; i < (int)callback_object->user_parameters.size(); i++) {
    internal_parameters[i + 1] = callback_object->user_parameters[i];
  }

  // Call php function with parameters.
  //
  // As in GObject_::connect_callback: a throwable must not unwind across
  // GLib's C frames, so it is captured here and reported only after the catch
  // scope has exited and released the pending Zend exception.
  std::string callback_error;
  long int callback_error_code = 0;
  bool callback_failed = false;
  try {
    Php::call("call_user_func_array", callback_name, internal_parameters);
  } catch (Php::Throwable &throwable) {
    callback_error = throwable.what();
    callback_error_code = throwable.code();
    callback_failed = true;
  }

  if (callback_failed) {
    phpgtk_report_callback_exception(callback_error, callback_error_code,
                                     "GtkClipboard::request_text");
  }

  // One-shot request: this is the only invocation, so the data allocated by
  // request_text() is released here.
  delete callback_object;
}

void GtkClipboard_::request_image(Php::Parameters &parameters) {
  // std::string s_user_function = parameters[0];
  // gchar *user_function = (gchar *)s_user_function.c_str();

  // gpointer user_data = (gpointer)parameters[1];

  // gtk_clipboard_request_image (GTK_CLIPBOARD(instance), user_function, user_data);

  throw Php::Exception("GtkClipboard_::request_image not implemented");
}

void GtkClipboard_::request_targets(Php::Parameters &parameters) {
  // std::string s_user_function = parameters[0];
  // gchar *user_function = (gchar *)s_user_function.c_str();

  // gpointer user_data = (gpointer)parameters[1];

  // gtk_clipboard_request_targets (GTK_CLIPBOARD(instance), user_function, user_data);

  throw Php::Exception("tkClipboard_::request_targets not implemented");
}

void GtkClipboard_::request_rich_text(Php::Parameters &parameters) {
  // std::string s_user_function = parameters[0];
  // gchar *user_function = (gchar *)s_user_function.c_str();

  // gpointer user_data = (gpointer)parameters[1];

  // gtk_clipboard_request_rich_text (GTK_CLIPBOARD(instance), user_function, user_data);

  throw Php::Exception("tkClipboard_::request_rich_text not implemented");
}

void GtkClipboard_::request_uris(Php::Parameters &parameters) {
  // std::string s_user_function = parameters[0];
  // gchar *user_function = (gchar *)s_user_function.c_str();

  // gpointer user_data = (gpointer)parameters[1];

  // gtk_clipboard_request_uris (GTK_CLIPBOARD(instance), user_function, user_data);

  throw Php::Exception("tkClipboard_::request_uris not implemented");
}

Php::Value GtkClipboard_::wait_for_contents(Php::Parameters &parameters) {
  // std::string s_target = parameters[0];
  // gchar *target = (gchar *)s_target.c_str();

  // GtkSelectionData *ret = gtk_clipboard_wait_for_contents (GTK_CLIPBOARD(instance), target);

  // GtkSelectionData_ *return_parsed = new GtkSelectionData_();
  // return_parsed->set_instance((gpointer *)ret);
  // return Php::Object("GtkSelectionData", return_parsed);

  throw Php::Exception("tkClipboard_::wait_for_contents not implemented");
  return -1;
}

Php::Value GtkClipboard_::wait_for_text() {
  std::string ret = gtk_clipboard_wait_for_text(GTK_CLIPBOARD(instance));

  return ret;
}

Php::Value GtkClipboard_::wait_for_image() {
  GdkPixbuf *ret = gtk_clipboard_wait_for_image(GTK_CLIPBOARD(instance));

  GdkPixbuf_ *return_parsed = new GdkPixbuf_();
  return_parsed->set_instance(ret);
  return Php::Object("GdkPixbuf", return_parsed);
}

Php::Value GtkClipboard_::wait_for_rich_text(Php::Parameters &parameters) {
  // GtkTextBuffer *buffer;
  // if(parameters.size() > 0) {
  // 	Php::Value object_buffer = parameters[0];
  // 	GtkTextBuffer_ *phpgtk_buffer = (GtkTextBuffer_ *)object_buffer.implementation();
  // 	buffer = GTK_TEXT_BUFFER(phpgtk_buffer->get_instance());
  // }

  // std::string s_target = parameters[1];
  // gchar *target = (gchar *)s_target.c_str();

  // guint8 ret = gtk_clipboard_wait_for_rich_text (GTK_CLIPBOARD(instance), buffer, target);

  // return ret;

  throw Php::Exception("GtkClipboard_::wait_for_rich_text not implemented");
  return -1;
}

Php::Value GtkClipboard_::wait_for_uris() {
  // std::string ret[] = gtk_clipboard_wait_for_uris (GTK_CLIPBOARD(instance));

  // return ret;

  throw Php::Exception("GtkClipboard_::wait_for_uris not implemented");
  return -1;
}

Php::Value GtkClipboard_::wait_is_text_available() {
  bool ret = gtk_clipboard_wait_is_text_available(GTK_CLIPBOARD(instance));

  return ret;
}

Php::Value GtkClipboard_::wait_is_image_available(Php::Parameters &parameters) {
  GtkTextBuffer *buffer;
  if (!parameters.empty()) {
    Php::Value object_buffer = parameters[0];
    GtkTextBuffer_ *phpgtk_buffer = (GtkTextBuffer_ *)object_buffer.implementation();
    buffer = GTK_TEXT_BUFFER(phpgtk_buffer->get_instance());
  }

  bool ret = gtk_clipboard_wait_is_rich_text_available(GTK_CLIPBOARD(instance), buffer);

  return ret;
}

Php::Value GtkClipboard_::wait_is_uris_available() {
  bool ret = gtk_clipboard_wait_is_uris_available(GTK_CLIPBOARD(instance));

  return ret;
}

Php::Value GtkClipboard_::wait_is_target_available(Php::Parameters &parameters) {
  // std::string s_target = parameters[0];
  // gchar *target = (gchar *)s_target.c_str();

  // gboolean ret = gtk_clipboard_wait_is_target_available (GTK_CLIPBOARD(instance), target);

  // return ret;

  throw Php::Exception("GtkClipboard_::wait_for_uris not implemented");
  return -1;
}

void GtkClipboard_::set_can_store() {
  gtk_clipboard_store(GTK_CLIPBOARD(instance));
}

Php::Value GtkClipboard_::get_selection() {
  GdkAtom atom = gtk_clipboard_get_selection(GTK_CLIPBOARD(instance));

  std::string ret = gdk_atom_name(atom);

  return ret;
}
