#ifndef _PHPGTK_H_
#define _PHPGTK_H_

    #include <phpcpp.h>
    #include <iostream>
    #include <gtk/gtk.h>
    
	#include "src/Gtk/GtkWidget.h"


	bool phpgtk_check_parameter(Php::Parameters &parameters, int param, Php::Type expected_type, bool required, const char *object_type);
	char *phpgtk_wrong_type_message(int param, Php::Type type_passed, Php::Type type_expected);
	std::string phpgtk_type_to_string(Php::Type type);
	Php::Value cobject_to_phpobject(gpointer *cobject);

	/**
	 * There is deliberately no generic vararg callback marshaller here.
	 *
	 * There used to be one (generic_callback / generic_st_callback). Because a
	 * GLib callback signature does not say where the user data sits, it walked
	 * the varargs casting each argument in turn to its own struct type and
	 * dereferencing it, looking for one whose first member happened to be a
	 * callable. For GtkTreeSelectionForeachFunc that meant reading a GtkTreePath
	 * and a GtkTreeIter as PHP values; it only worked because arbitrary memory
	 * rarely looks like a callable.
	 *
	 * Write a callback typed to the exact GTK signature instead - the user data
	 * position is then known rather than guessed. See
	 * selected_foreach_callback() in src/Gtk/GtkTreeSelection.cpp for the shape.
	 */

	/**
	 * Reporting of PHP exceptions that escape a callback.
	 *
	 * A throwable must never unwind across GLib's C signal-emission frames, so
	 * every callback catches it and hands the details here instead. Reporting
	 * has to keep working when no PHP handler is installed, which is why the
	 * handler is *registered* rather than looked up by name: asking PHP whether
	 * an unknown callable exists can itself raise, and a pending Zend exception
	 * makes every later Php::call() a silent no-op.
	 *
	 * See Gtk_::set_exception_handler() for the PHP-facing API.
	 */
	void phpgtk_set_exception_handler(const Php::Value &handler);
	bool phpgtk_has_exception_handler();
	void phpgtk_report_callback_exception(const std::string &message, long int code,
	                                      const char *context);

	/**
	 * Build metadata ("built <date>, git <hash>") and the compiled-in optional
	 * features ("webkit=yes, gladeui=no, ..."), defined in version.cpp - the
	 * one translation unit the Makefile force-rebuilds so the values stay current.
	 */
	const char *phpgtk_build_info();
	const char *phpgtk_build_features();
	const char *phpgtk_phpcpp_info();


#endif