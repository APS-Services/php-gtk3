
#include "GtkAboutDialog.h"

/**
 * Build the NULL-terminated string list that GTK's credit setters expect.
 *
 * Accepts either a single string or an array of strings, so both
 * $dialog->set_authors("Alice") and $dialog->set_authors(["Alice", "Bob"])
 * work - GTK has always taken a list here, the binding just could not express
 * one.
 *
 * The returned pointers borrow from `storage`, which the caller must keep alive
 * until the GTK call has returned. GTK copies the strings itself, so nothing has
 * to outlive the setter.
 */
static std::vector<const gchar *> phpgtk_credit_list(const Php::Value &value,
                                                     std::vector<std::string> &storage) {
  if (value.isArray()) {
    for (const auto &entry : value) {
      storage.push_back(entry.second.stringValue());
    }
  } else {
    storage.push_back(value.stringValue());
  }

  // Filled only once storage is complete: push_back may reallocate, which would
  // invalidate any c_str() taken before the last insertion.
  std::vector<const gchar *> list;
  list.reserve(storage.size() + 1);
  for (const std::string &entry : storage) {
    list.push_back(entry.c_str());
  }
  list.push_back(nullptr);

  return list;
}

/**
 * Turn one of GTK's NULL-terminated credit lists into a PHP array.
 *
 * The list belongs to the dialog: the strings are copied into the returned
 * value and nothing is freed here. A NULL list (nothing set yet) yields an
 * empty array.
 */
static Php::Value phpgtk_credit_array(const gchar *const *list) {
  Php::Array ret;

  for (int i = 0; (list != nullptr) && (list[i] != nullptr); i++) {
    ret[i] = list[i];
  }

  return ret;
}

/**
 * Constructor
 */
GtkAboutDialog_::GtkAboutDialog_() = default;

/**
 * Destructor
 */
GtkAboutDialog_::~GtkAboutDialog_() = default;

void GtkAboutDialog_::__construct() {
  instance = (gpointer *)gtk_about_dialog_new();
}

Php::Value GtkAboutDialog_::get_program_name() {
  std::string ret = gtk_about_dialog_get_program_name(GTK_ABOUT_DIALOG(instance));

  return ret;
}

void GtkAboutDialog_::set_program_name(Php::Parameters &parameters) {
  std::string s_name = parameters[0];
  gchar *name = (gchar *)s_name.c_str();

  gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(instance), name);
}

Php::Value GtkAboutDialog_::get_version() {
  std::string ret = gtk_about_dialog_get_version(GTK_ABOUT_DIALOG(instance));

  return ret;
}

void GtkAboutDialog_::set_version(Php::Parameters &parameters) {
  std::string s_version = parameters[0];
  gchar *version = (gchar *)s_version.c_str();

  gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(instance), version);
}

Php::Value GtkAboutDialog_::get_copyright() {
  std::string ret = gtk_about_dialog_get_copyright(GTK_ABOUT_DIALOG(instance));

  return ret;
}

void GtkAboutDialog_::set_copyright(Php::Parameters &parameters) {
  std::string s_copyright = parameters[0];
  gchar *copyright = (gchar *)s_copyright.c_str();

  gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(instance), copyright);
}

Php::Value GtkAboutDialog_::get_comments() {
  std::string ret = gtk_about_dialog_get_comments(GTK_ABOUT_DIALOG(instance));

  return ret;
}

void GtkAboutDialog_::set_comments(Php::Parameters &parameters) {
  std::string s_comments = parameters[0];
  gchar *comments = (gchar *)s_comments.c_str();

  gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(instance), comments);
}

Php::Value GtkAboutDialog_::get_license() {
  std::string ret = gtk_about_dialog_get_license(GTK_ABOUT_DIALOG(instance));

  return ret;
}

void GtkAboutDialog_::set_license(Php::Parameters &parameters) {
  std::string s_license = parameters[0];
  gchar *license = (gchar *)s_license.c_str();

  gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(instance), license);
}

Php::Value GtkAboutDialog_::get_wrap_license() {
  bool ret = gtk_about_dialog_get_wrap_license(GTK_ABOUT_DIALOG(instance));

  return ret;
}

void GtkAboutDialog_::set_wrap_license(Php::Parameters &parameters) {
  gboolean wrap_license = (gboolean)parameters[0];

  gtk_about_dialog_set_wrap_license(GTK_ABOUT_DIALOG(instance), wrap_license);
}

Php::Value GtkAboutDialog_::get_license_type() {
  GtkLicense ret = gtk_about_dialog_get_license_type(GTK_ABOUT_DIALOG(instance));

  return ret;
}

void GtkAboutDialog_::set_license_type(Php::Parameters &parameters) {
  int int_license_type = (int)parameters[0];
  GtkLicense license_type = (GtkLicense)int_license_type;

  gtk_about_dialog_set_license_type(GTK_ABOUT_DIALOG(instance), license_type);
}

Php::Value GtkAboutDialog_::get_website() {
  std::string ret = gtk_about_dialog_get_website(GTK_ABOUT_DIALOG(instance));

  return ret;
}

void GtkAboutDialog_::set_website(Php::Parameters &parameters) {
  std::string s_website = parameters[0];
  gchar *website = (gchar *)s_website.c_str();

  gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(instance), website);
}

Php::Value GtkAboutDialog_::get_website_label() {
  std::string ret = gtk_about_dialog_get_website_label(GTK_ABOUT_DIALOG(instance));

  return ret;
}

void GtkAboutDialog_::set_website_label(Php::Parameters &parameters) {
  std::string s_website_label = parameters[0];
  gchar *website_label = (gchar *)s_website_label.c_str();

  gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(instance), website_label);
}

Php::Value GtkAboutDialog_::get_authors() {
  return phpgtk_credit_array(gtk_about_dialog_get_authors(GTK_ABOUT_DIALOG(instance)));
}

void GtkAboutDialog_::set_authors(Php::Parameters &parameters) {
  // Local, not static: the list holds pointers into storage, which dies with
  // this call. A static list would be initialised on the first call only and
  // keep handing that first (long dangling) pointer to GTK on every later call.
  std::vector<std::string> storage;
  std::vector<const gchar *> authors = phpgtk_credit_list(parameters[0], storage);

  gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(instance), authors.data());
}

Php::Value GtkAboutDialog_::get_artists() {
  return phpgtk_credit_array(gtk_about_dialog_get_artists(GTK_ABOUT_DIALOG(instance)));
}

void GtkAboutDialog_::set_artists(Php::Parameters &parameters) {
  // Local, not static - see set_authors() for why.
  std::vector<std::string> storage;
  std::vector<const gchar *> artists = phpgtk_credit_list(parameters[0], storage);

  gtk_about_dialog_set_artists(GTK_ABOUT_DIALOG(instance), artists.data());
}

Php::Value GtkAboutDialog_::get_documenters() {
  return phpgtk_credit_array(gtk_about_dialog_get_documenters(GTK_ABOUT_DIALOG(instance)));
}

void GtkAboutDialog_::set_documenters(Php::Parameters &parameters) {
  // Local, not static - see set_authors() for why.
  std::vector<std::string> storage;
  std::vector<const gchar *> documenters = phpgtk_credit_list(parameters[0], storage);

  gtk_about_dialog_set_documenters(GTK_ABOUT_DIALOG(instance), documenters.data());
}

Php::Value GtkAboutDialog_::get_translator_credits() {
  std::string ret = gtk_about_dialog_get_translator_credits(GTK_ABOUT_DIALOG(instance));

  return ret;
}

void GtkAboutDialog_::set_translator_credits(Php::Parameters &parameters) {
  std::string s_translator_credits = parameters[0];
  gchar *translator_credits = (gchar *)s_translator_credits.c_str();

  gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(instance), translator_credits);
}

Php::Value GtkAboutDialog_::get_logo() {
  GdkPixbuf *l_pixbuf = gtk_about_dialog_get_logo(GTK_ABOUT_DIALOG(instance));

  // Create the PHP-GTK object and set GTK object
  GdkPixbuf_ *pixbuf_ = new GdkPixbuf_();
  pixbuf_->set_instance(l_pixbuf);

  // Return PHP-GTK object
  return Php::Object("GdkPixbuf", pixbuf_);
}

void GtkAboutDialog_::set_logo(Php::Parameters &parameters) {
  GdkPixbuf *logo;
  if (!parameters.empty()) {
    Php::Value object_logo = parameters[0];
    GdkPixbuf_ *phpgtk_logo = (GdkPixbuf_ *)object_logo.implementation();
    logo = phpgtk_logo->get_instance();
  }

  gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(instance), logo);
}

Php::Value GtkAboutDialog_::get_logo_icon_name() {
  std::string ret = gtk_about_dialog_get_logo_icon_name(GTK_ABOUT_DIALOG(instance));

  return ret;
}

void GtkAboutDialog_::set_logo_icon_name(Php::Parameters &parameters) {
  std::string s_icon_name = parameters[0];
  gchar *icon_name = (gchar *)s_icon_name.c_str();

  gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(instance), icon_name);
}

void GtkAboutDialog_::add_credit_section(Php::Parameters &parameters) {
  std::string s_section_name = parameters[0];
  gchar *section_name = (gchar *)s_section_name.c_str();

  // Local, not static - see set_authors() for why.
  std::vector<std::string> storage;
  std::vector<const gchar *> people = phpgtk_credit_list(parameters[1], storage);

  gtk_about_dialog_add_credit_section(GTK_ABOUT_DIALOG(instance), section_name, people.data());
}

void GtkAboutDialog_::gtk_show_about_dialog(Php::Parameters &parameters) {
  // https://stackoverflow.com/questions/21693960/gtkaboutdialog-icon-not-loading

  // std::string s_first_property_name = parameters[0];
  // gchar *first_property_name = (gchar *)s_first_property_name.c_str();

  // gtk_show_about_dialog (GTK_ABOUT_DIALOG(instance), first_property_name, );

  throw Php::Exception("GtkAboutDialog_::gtk_show_about_dialog not implemented");
}
