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
	 * Struct for generic callback
	 *
	 * The parameters are kept as a plain vector rather than a Php::Parameters:
	 * Php::Parameters has no public default constructor, so a struct holding one
	 * cannot be constructed normally - only the (invalid) malloc + memset trick
	 * this struct used to be created with would compile.
	 */
	struct generic_st_callback {
		Php::Value callback_name;
		Php::Object self_widget;
		std::vector<Php::Value> parameters;

		GType return_type;
		int n_params;
		GType *param_types;

		// Where this callback was installed (e.g. "GtkTreeSelection::selected_foreach"),
		// used when reporting an exception that escaped the PHP callback. Must point
		// to a string literal - the struct outlives the installing call.
		const char *context;
	};

	void generic_callback(gpointer *self, ...);

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