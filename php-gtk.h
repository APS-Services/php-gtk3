#ifndef _PHPGTK_H_
#define _PHPGTK_H_

    #include <phpcpp.h>
    #include <iostream>
    #include <gtk/gtk.h>
    #include <cstdarg>

	#include "src/Gtk/GtkWidget.h"


	bool phpgtk_check_parameter(Php::Parameters &parameters, int param, Php::Type expected_type, bool required, const char *object_type);
	char *phpgtk_wrong_type_message(int param, Php::Type type_passed, Php::Type type_expected);
	std::string phpgtk_type_to_string(Php::Type type);
	Php::Value cobject_to_phpobject(gpointer *cobject);

	/**
	 * Struct for generic callback
	 */
	struct generic_st_callback {
		Php::Value callback_name;
		Php::Object self_widget;
		// Not Php::Parameters: it has no default constructor, and the struct is created with new
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
	 * Calls va_end() when it leaves scope.
	 *
	 * The callback trampolines wrap their whole body - marshalling included - in
	 * a try block, because building a Php::Object from a GType that main.cpp does
	 * not register throws. A plain va_end() after the marshalling loop would be
	 * skipped when that happens, and va_end() has to run in the same function
	 * that called va_start(), which a local guard satisfies.
	 */
	class phpgtk_va_list_guard {
	 public:
		explicit phpgtk_va_list_guard(va_list &list) : _list(list) {}
		~phpgtk_va_list_guard() { va_end(_list); }

		phpgtk_va_list_guard(const phpgtk_va_list_guard &) = delete;
		phpgtk_va_list_guard &operator=(const phpgtk_va_list_guard &) = delete;

	 private:
		va_list &_list;
	};

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
	 * exit()/die() inside a callback.
	 *
	 * Since PHP 8, exit() no longer bails out on the spot: it throws an internal
	 * "unwind exit" that PHP-CPP hands to the callback's catch like any other
	 * throwable, and clearing it there would silently cancel the exit - the
	 * application keeps running after it asked to terminate. Call
	 * phpgtk_exit_pending() inside the catch (while the exception is still
	 * pending) and phpgtk_finish_exit() after the catch scope has been left.
	 */
	bool phpgtk_exit_pending();
	[[noreturn]] void phpgtk_finish_exit();

	/**
	 * GLib out-parameter errors.
	 *
	 * A GError has to be NULL before a GLib call fills it in - GLib reads it to
	 * see whether an error is already set - and has to be freed afterwards, or
	 * the message leaks. Both are easy to get wrong the same way in every
	 * binding, so declare the error as `GError *error = nullptr;` and hand it to
	 * whichever of these fits the call:
	 *
	 *   phpgtk_warn_on_error()  reports it as a PHP warning and frees it. For a
	 *                           call that already tells PHP it failed, through a
	 *                           false return value, so losing only the reason.
	 *   phpgtk_throw_on_error() frees it and throws. For a call whose return
	 *                           value cannot say "it failed" - one handing back
	 *                           an object, where the alternative is wrapping a
	 *                           NULL that crashes on first use.
	 *
	 * Both do nothing when no error was set. 'call' names the binding in the
	 * message ("GtkCssProvider::load_from_data") and must outlive the call, so
	 * pass a string literal.
	 */
	void phpgtk_warn_on_error(const char *call, GError *&error);
	void phpgtk_throw_on_error(const char *call, GError *&error);

	/**
	 * Build metadata ("built <date>, git <hash>") and the compiled-in optional
	 * features ("webkit=yes, gladeui=no, ..."), defined in version.cpp - the
	 * one translation unit the Makefile force-rebuilds so the values stay current.
	 */
	const char *phpgtk_build_info();
	const char *phpgtk_build_features();
	const char *phpgtk_phpcpp_info();


#endif