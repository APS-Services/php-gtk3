/**
 * PHP-CPP
 *
 * Sobre como extender classes 
 *      https://github.com/CopernicaMarketingSoftware/PHP-CPP/issues/211
 *
 * 
 */


#include <phpcpp.h>
#include <iostream>

#include <gtk/gtk.h>

#include "src/Gtk.h"
#include "src/GtkWidget.h"
#include "src/GtkWindow.h"

// https://www.sitepoint.com/developing-php-extensions-c-php-cpp-advanced/


/**
 *  tell the compiler that the get_module is a pure C function
 */
extern "C"
{

    /**
     *  Function that is called by PHP right after the PHP process
     *  has started, and that returns an address of an internal PHP
     *  strucure with all the details and features of your extension
     *
     *  @return void*   a pointer to an address that is understood by PHP
     */
    PHPCPP_EXPORT void *get_module() 
    {
        // static(!) Php::Extension object that should stay in memory
        // for the entire duration of the process (that's why it's static)
        static Php::Extension extension("php-gtk3", "1.0");

        // Initialize GTK
        gtk_init (0, NULL);

        // Gtk
        Php::Class<Gtk_> gtk("Gtk");
            gtk.method<&Gtk_::main>("main");
            gtk.method<&Gtk_::main_quit>("main_quit");
            

        // GdkEvent
        Php::Class<GdkEvent_> gdkevent("GdkEvent");
            // gdkevent.method<&GdkEvent_::__construct>("__construct");
            // gdkevent.property("type", 0);
            

        // GtkWidget
        Php::Class<GtkWidget_> gtkwidget("GtkWidget");
            gtkwidget.method<&GtkWidget_::show_all>("show_all");
            gtkwidget.method<&GtkWidget_::connect>("connect");
            gtkwidget.method<&GtkWidget_::handler_disconnect>("handler_disconnect");
            // gtkwidget.method<&GtkWidget_::test>("test");
            

        // GtkWindow
        Php::Class<GtkWindow_> gtkwindow("GtkWindow");
            gtkwindow.extends(gtkwidget);
            

        // Add
        extension.add(std::move(gtk));
        extension.add(std::move(gdkevent));
        extension.add(std::move(gtkwidget));
        extension.add(std::move(gtkwindow));


        // return the extension
        return extension;
    }
}