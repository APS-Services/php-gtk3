

#include "GtkStatusIcon.h"

// Helper function to ensure GTK is initialized
// GtkStatusIcon requires a display connection for proper operation
static void ensure_gtk_initialized()
{
	// Check if GTK has been initialized by checking if there's a default display
	if (gdk_display_get_default() == NULL) {
		// GTK not initialized, initialize it now
		// We use gtk_init_check to avoid aborting if initialization fails
		int argc = 0;
		char **argv = NULL;
		gtk_init_check(&argc, &argv);
	}
}

GtkStatusIcon_::GtkStatusIcon_() = default;
GtkStatusIcon_::~GtkStatusIcon_() = default;

void GtkStatusIcon_::set_from_pixbuf(Php::Parameters &parameters)
{
	GdkPixbuf *pixbuf;
	Php::Value object_pixbuf = parameters[0];
	GdkPixbuf_ *phpgtk_pixbuf = (GdkPixbuf_ *)object_pixbuf.implementation();
	pixbuf = GDK_PIXBUF(phpgtk_pixbuf->get_instance());

	gtk_status_icon_set_from_pixbuf(GTK_STATUS_ICON(instance), pixbuf);
}

void GtkStatusIcon_::set_from_file(Php::Parameters &parameters)
{
	std::string c_filename = parameters[0];

	gchar *filename = (gchar *)c_filename.c_str();

	gtk_status_icon_set_from_file(GTK_STATUS_ICON(instance), filename);
}

void GtkStatusIcon_::set_from_stock(Php::Parameters &parameters)
{
	std::string c_stock_id = parameters[0];

	gchar *stock_id = (gchar *)c_stock_id.c_str();

	gtk_status_icon_set_from_stock(GTK_STATUS_ICON(instance), stock_id);
}

void GtkStatusIcon_::set_from_icon_name(Php::Parameters &parameters)
{
	std::string c_icon_name = parameters[0];

	gchar *icon_name = (gchar *)c_icon_name.c_str();

	gtk_status_icon_set_from_icon_name(GTK_STATUS_ICON(instance), icon_name);
}

/*
Php::Value GtkStatusIcon_::get_storage_type()
{
	// GtkImageType ret = gtk_status_icon_get_storage_type(GTK_STATUS_ICON(instance));

	// GtkImageType_ *phpgtk_ret = new GtkImageType_();
	// phpgtk_ret->set_instance((gpointer *)ret);
	// return Php::Object("GtkImageType", phpgtk_ret);
}
*/

Php::Value GtkStatusIcon_::get_pixbuf()
{
	GdkPixbuf* ret = gtk_status_icon_get_pixbuf(GTK_STATUS_ICON(instance));

	GdkPixbuf_ *phpgtk_ret = new GdkPixbuf_();
	phpgtk_ret->set_instance(ret);
	return Php::Object("GdkPixbuf", phpgtk_ret);
}

Php::Value GtkStatusIcon_::get_stock()
{
	const gchar* ret = gtk_status_icon_get_stock(GTK_STATUS_ICON(instance));

	return ret;
}

Php::Value GtkStatusIcon_::get_icon_name()
{
	const gchar* ret = gtk_status_icon_get_icon_name(GTK_STATUS_ICON(instance));

	return ret;
}

/*
Php::Value GtkStatusIcon_::get_gicon()
{
	// GIcon* ret = gtk_status_icon_get_gicon(GTK_STATUS_ICON(instance));

	// GIcon*_ *phpgtk_ret = new GIcon*_();
	// phpgtk_ret->set_instance((gpointer *)ret);
	// return Php::Object("GIcon*", phpgtk_ret);
}
*/

Php::Value GtkStatusIcon_::get_size()
{
	gint ret = gtk_status_icon_get_size(GTK_STATUS_ICON(instance));

	return ret;
}

Php::Value GtkStatusIcon_::get_screen()
{
	GdkScreen* ret = gtk_status_icon_get_screen(GTK_STATUS_ICON(instance));

	GdkScreen_ *phpgtk_ret = new GdkScreen_();
	phpgtk_ret->set_instance(ret);
	return Php::Object("GdkScreen", phpgtk_ret);
}

void GtkStatusIcon_::set_has_tooltip(Php::Parameters &parameters)
{
	gboolean has_tooltip = (gboolean)parameters[0];

	gtk_status_icon_set_has_tooltip(GTK_STATUS_ICON(instance), has_tooltip);
}

void GtkStatusIcon_::set_tooltip_text(Php::Parameters &parameters)
{
	std::string c_text = parameters[0];

	gchar *text = (gchar *)c_text.c_str();

	gtk_status_icon_set_tooltip_text(GTK_STATUS_ICON(instance), text);
}

void GtkStatusIcon_::set_tooltip_markup(Php::Parameters &parameters)
{
	std::string c_markup = parameters[0];

	gchar *markup = (gchar *)c_markup.c_str();

	gtk_status_icon_set_tooltip_markup(GTK_STATUS_ICON(instance), markup);
}

void GtkStatusIcon_::set_title(Php::Parameters &parameters)
{
	std::string c_title = parameters[0];

	gchar *title = (gchar *)c_title.c_str();

	gtk_status_icon_set_title(GTK_STATUS_ICON(instance), title);
}

Php::Value GtkStatusIcon_::get_title()
{
	const gchar* ret = gtk_status_icon_get_title(GTK_STATUS_ICON(instance));

	return ret;
}

void GtkStatusIcon_::set_name(Php::Parameters &parameters)
{
	std::string c_name = parameters[0];

	gchar *name = (gchar *)c_name.c_str();

	gtk_status_icon_set_name(GTK_STATUS_ICON(instance), name);
}

void GtkStatusIcon_::set_visible(Php::Parameters &parameters)
{
	gboolean visible = (gboolean)parameters[0];

	gtk_status_icon_set_visible(GTK_STATUS_ICON(instance), visible);
}

Php::Value GtkStatusIcon_::get_visible()
{
	bool ret = gtk_status_icon_get_visible(GTK_STATUS_ICON(instance));

	return ret;
}

Php::Value GtkStatusIcon_::is_embedded()
{
	bool ret = gtk_status_icon_is_embedded(GTK_STATUS_ICON(instance));

	return ret;
}

Php::Value GtkStatusIcon_::get_has_tooltip()
{
	bool ret = gtk_status_icon_get_has_tooltip(GTK_STATUS_ICON(instance));

	return ret;
}

Php::Value GtkStatusIcon_::get_tooltip_text()
{
	gchar* ret = gtk_status_icon_get_tooltip_text(GTK_STATUS_ICON(instance));

	return ret;
}

Php::Value GtkStatusIcon_::get_tooltip_markup()
{
	gchar* ret = gtk_status_icon_get_tooltip_markup(GTK_STATUS_ICON(instance));

	return ret;
}

/*
Php::Value GtkStatusIcon_::get_x11_window_id()
{
	// guint32 ret = gtk_status_icon_get_x11_window_id(GTK_STATUS_ICON(instance));

	// guint32_ *phpgtk_ret = new guint32_();
	// phpgtk_ret->set_instance((gpointer *)ret);
	// return Php::Object("guint32", phpgtk_ret);
}
*/

void GtkStatusIcon_::__construct()
{
	ensure_gtk_initialized();
	instance = (gpointer *)gtk_status_icon_new();
}

Php::Value GtkStatusIcon_::new_from_pixbuf(Php::Parameters &parameters)
{
	GdkPixbuf *pixbuf;
	Php::Value object_pixbuf = parameters[0];
	GdkPixbuf_ *phpgtk_pixbuf = (GdkPixbuf_ *)object_pixbuf.implementation();
	pixbuf = GDK_PIXBUF(phpgtk_pixbuf->get_instance());

	ensure_gtk_initialized();
	GtkStatusIcon* ret = gtk_status_icon_new_from_pixbuf(pixbuf);

	GtkStatusIcon_ *phpgtk_ret = new GtkStatusIcon_();
	phpgtk_ret->set_instance((gpointer *)ret);
	return Php::Object("GtkStatusIcon", phpgtk_ret);
}

Php::Value GtkStatusIcon_::new_from_file(Php::Parameters &parameters)
{
	static int debug_counter = 0;
	
	fprintf(stderr, "[DEBUG %d] new_from_file: Start\n", ++debug_counter);
	fflush(stderr);
	
	std::string c_filename = parameters[0];
	fprintf(stderr, "[DEBUG %d] new_from_file: Got filename parameter\n", ++debug_counter);
	fflush(stderr);
	
	gchar *filename = (gchar *)c_filename.c_str();
	fprintf(stderr, "[DEBUG %d] new_from_file: Converted to gchar*, filename=%s\n", ++debug_counter, filename);
	fflush(stderr);

	fprintf(stderr, "[DEBUG %d] new_from_file: About to call ensure_gtk_initialized()\n", ++debug_counter);
	fflush(stderr);
	
	ensure_gtk_initialized();
	fprintf(stderr, "[DEBUG %d] new_from_file: GTK initialized\n", ++debug_counter);
	fflush(stderr);
	
	fprintf(stderr, "[DEBUG %d] new_from_file: About to call gtk_status_icon_new_from_file()\n", ++debug_counter);
	fflush(stderr);
	
	GtkStatusIcon* ret = gtk_status_icon_new_from_file(filename);
	fprintf(stderr, "[DEBUG %d] new_from_file: gtk_status_icon_new_from_file() returned, ret=%p\n", ++debug_counter, (void*)ret);
	fflush(stderr);

	fprintf(stderr, "[DEBUG %d] new_from_file: About to create GtkStatusIcon_ wrapper\n", ++debug_counter);
	fflush(stderr);
	
	GtkStatusIcon_ *phpgtk_ret = new GtkStatusIcon_();
	fprintf(stderr, "[DEBUG %d] new_from_file: Created wrapper, phpgtk_ret=%p\n", ++debug_counter, (void*)phpgtk_ret);
	fflush(stderr);
	
	phpgtk_ret->set_instance((gpointer *)ret);
	fprintf(stderr, "[DEBUG %d] new_from_file: Set instance\n", ++debug_counter);
	fflush(stderr);
	
	fprintf(stderr, "[DEBUG %d] new_from_file: About to return Php::Object\n", ++debug_counter);
	fflush(stderr);
	
	return Php::Object("GtkStatusIcon", phpgtk_ret);
}

Php::Value GtkStatusIcon_::new_from_stock(Php::Parameters &parameters)
{
	std::string c_stock_id = parameters[0];
	gchar *stock_id = (gchar *)c_stock_id.c_str();

	ensure_gtk_initialized();
	GtkStatusIcon* ret = gtk_status_icon_new_from_stock(stock_id);

	GtkStatusIcon_ *phpgtk_ret = new GtkStatusIcon_();
	phpgtk_ret->set_instance((gpointer *)ret);
	return Php::Object("GtkStatusIcon", phpgtk_ret);
}

Php::Value GtkStatusIcon_::new_from_icon_name(Php::Parameters &parameters)
{
	std::string c_icon_name = parameters[0];
	gchar *icon_name = (gchar *)c_icon_name.c_str();

	ensure_gtk_initialized();
	GtkStatusIcon* ret = gtk_status_icon_new_from_icon_name(icon_name);

	GtkStatusIcon_ *phpgtk_ret = new GtkStatusIcon_();
	phpgtk_ret->set_instance((gpointer *)ret);
	return Php::Object("GtkStatusIcon", phpgtk_ret);
}


Php::Value GtkStatusIcon_::position_menu(Php::Parameters &parameters)
{
	GtkMenu *menu;
	Php::Value object_menu = parameters[0];
	GtkMenu_ *phpgtk_menu = (GtkMenu_ *)object_menu.implementation();
	menu = GTK_MENU(phpgtk_menu->get_instance());

	gint* x;
	gint* y;
	gboolean *push_in;

	gtk_status_icon_position_menu(menu, x, y, push_in, GTK_STATUS_ICON(instance));

	int64_t ux = (int64_t)&x;
	int64_t uy = (int64_t)&y;

    Php::Value arr;
    arr["x"] = ux;
    arr["y"] = uy;

    return arr;
}




