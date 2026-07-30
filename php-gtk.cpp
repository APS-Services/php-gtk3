
#include "php-gtk.h"

/**
 * Slot holding the handler installed from PHP via Gtk::set_exception_handler(),
 * or a null Php::Value when the application did not install one.
 *
 * Deliberately allocated once and never freed: a Php::Value with static storage
 * duration would be destroyed at process exit, i.e. after Zend has already shut
 * down, and releasing a zval at that point is not safe. Reassigning the slot
 * instead releases the previous callable while PHP is still up.
 */
static Php::Value &phpgtk_exception_handler() {
  static Php::Value *handler = new Php::Value();
  return *handler;
}

/**
 * Install (or, with a null value, remove) the callback exception handler.
 */
void phpgtk_set_exception_handler(const Php::Value &handler) {
  phpgtk_exception_handler() = handler;
}

/**
 * Whether a handler is installed. This only inspects the stored zval's type on
 * purpose - it must not call into PHP, because it runs on a path where an
 * exception may still be pending and every call would be dropped.
 */
bool phpgtk_has_exception_handler() {
  return !phpgtk_exception_handler().isNull();
}

/**
 * Report an exception that escaped a PHP callback.
 *
 * Call this only *after* the catch block that captured the throwable has been
 * left: PHP-CPP clears the pending Zend exception when the caught object is
 * destroyed, and while it is still pending Zend refuses to run any function,
 * which would silently swallow both the handler and the fallback.
 *
 * Falls back to g_critical(), which is plain C and cannot throw, so a failure
 * is always visible even when no handler is installed or the handler itself
 * fails.
 */
void phpgtk_report_callback_exception(const std::string &message, long int code,
                                      const char *context) {
  // Signal name for signal handlers, otherwise the installing method
  // (e.g. "Gtk::timeout_add")
  const char *origin = (context != nullptr) ? context : "";
  bool reported = false;

  if (phpgtk_has_exception_handler()) {
    try {
      Php::call("call_user_func", phpgtk_exception_handler(), message, std::string(origin),
                static_cast<int64_t>(code));
      reported = true;
    } catch (...) {
      // The handler itself failed; fall through to g_critical() below so the
      // original exception is still reported. Nothing may escape into GLib.
      g_critical(
          "php-gtk3: Gtk::set_exception_handler() callback failed while reporting an "
          "exception from the '%s' handler",
          origin);
    }
  }

  if (!reported) {
    g_critical("php-gtk3: uncaught exception in '%s' handler: %s", origin, message.c_str());
  }
}

bool phpgtk_check_parameter(Php::Parameters &parameters, int param, Php::Type expected_type,
                            bool required, const char *object_type) {
  int param_count = parameters.size();

  if (required) {
    // ----
    if (param_count < param) {
      throw Php::Exception(std::string("Missing required parameter ") + std::to_string(param));
    }

    // Test boolean individually cause True and False are different types
    if ((expected_type == Php::Type::Bool) &&
        ((parameters[param - 1].type() == Php::Type::True) ||
         (parameters[param - 1].type() == Php::Type::False))) {
      return true;
    }

    // Text Object type
    if ((expected_type == Php::Type::Object) && (!Php::is_a(parameters[param - 1], object_type))) {
      throw Php::Exception(
          phpgtk_wrong_type_message(param, parameters[param - 1].type(), expected_type));
    }

    // ----
    if (parameters[param - 1].type() != expected_type) {
      throw Php::Exception(
          phpgtk_wrong_type_message(param, parameters[param - 1].type(), expected_type));
    }

  } else {
    // ----
    if (param_count < param) {
      return false;
    }

    // Test boolean individually cause True and False are different types
    if ((expected_type == Php::Type::Bool) &&
        ((parameters[param - 1].type() == Php::Type::True) ||
         (parameters[param - 1].type() == Php::Type::False))) {
      return true;
    }

    // Test Object type before general type check
    if ((expected_type == Php::Type::Object) && (!Php::is_a(parameters[param - 1], object_type))) {
      Php::warning << "Invalid type for optional parameter " << param << std::flush;
      return false;
    }

    // General type check
    if (parameters[param - 1].type() != expected_type) {
      Php::warning << phpgtk_wrong_type_message(param, parameters[param - 1].type(), expected_type)
                   << std::flush;
      return false;
    }
  }

  return true;
}

/**
 *
 */
char *phpgtk_wrong_type_message(int param, Php::Type type_passed, Php::Type type_expected) {
  char *buffer;
  int len;

  // Get len of string
  len = snprintf(nullptr, 0, "Expected parameter %d to be an %s, %s given", param,
                 phpgtk_type_to_string(type_expected).c_str(),
                 phpgtk_type_to_string(type_passed).c_str());
  buffer = (char *)malloc((len + 1) * sizeof(char));

  // Save into buffer
  snprintf(buffer, len + 1, "Expected parameter %d to be an %s, %s given", param,
           phpgtk_type_to_string(type_expected).c_str(),
           phpgtk_type_to_string(type_passed).c_str());
  Php::call("echo", buffer);
  // Php::call("var_dump", phpgtk_type_to_string(type_passed).c_str());
  // Php::call("var_dump", phpgtk_type_to_string(type_expected).c_str());

  // throw
  return buffer;
}

/**
 *
 */
std::string phpgtk_type_to_string(Php::Type type) {
  std::string str_type;

  switch (type) {
    case Php::Type::Undefined:
      str_type.assign("undefined");
      break;
    case Php::Type::Null:
      str_type.assign("null");
      break;
    case Php::Type::False:
      str_type.assign("false");
      break;
    case Php::Type::True:
      str_type.assign("true");
      break;
    case Php::Type::Numeric:
      str_type.assign("numeric");
      break;
    case Php::Type::Float:
      str_type.assign("float");
      break;
    case Php::Type::String:
      str_type.assign("string");
      break;
    case Php::Type::Array:
      str_type.assign("array");
      break;
    case Php::Type::Object:
      str_type.assign("object");
      break;
    case Php::Type::Resource:
      str_type.assign("resource");
      break;
    case Php::Type::Reference:
      str_type.assign("reference");
      break;
    case Php::Type::Constant:
      str_type.assign("constant");
      break;
    case Php::Type::ConstantAST:
      str_type.assign("constantAST");
      break;
    case Php::Type::Bool:
      str_type.assign("bool");
      break;
    case Php::Type::Callable:
      str_type.assign("callable");
      break;
  }

  return str_type;
}

Php::Value cobject_to_phpobject(gpointer *cobject) {
  if (cobject == nullptr) {
    return {};
  }

  GtkWidget_ *return_parsed = new GtkWidget_();
  return_parsed->set_instance((gpointer *)cobject);
  return Php::Object(g_type_name(G_TYPE_FROM_INSTANCE((gpointer *)cobject)), return_parsed);
}

/**
 * generict callback
 *
 * Assume that all functions pass self widget as first param
 */
void generic_callback(gpointer *self, ...) {
  /**
   *  This will loop all vargs after *self, max to 5 times, casting to generic_st_callback
   *  	if the generic_st_callback.callback_name is a php land user function, this will be callable,
   * so we know that this param is the userdata
   */
  struct generic_st_callback *callback_object;

  va_list ap;
  va_start(ap, self);
  bool test = false;
  for (int i = 0; i < 5; i++) {
    callback_object = (struct generic_st_callback *)va_arg(ap, generic_st_callback *);
    if (callback_object->callback_name.isCallable()) {
      va_end(ap);
      test = true;
      break;
    }
  }
  va_end(ap);

  if (!test) {
    std::string error;
    throw Php::Exception(error + "cannot find callable method");
  }

  // create internal params
  Php::Value internal_parameters;

  // add self widget
  internal_parameters[0] = cobject_to_phpobject((gpointer *)self);

  // Loop into param_types of GSignalQuery from g_signal_query
  va_start(ap, self);
  for (int i = 1; i < callback_object->n_params; i++) {
    switch (G_TYPE_FUNDAMENTAL(callback_object->param_types[i])) {
      case G_TYPE_CHAR:
        // Php::call("var_dump", "char");
        break;

      case G_TYPE_UCHAR:
        // Php::call("var_dump", "uchar");
        break;

      case G_TYPE_STRING:
        // Php::call("var_dump", "string");
        internal_parameters[i] = va_arg(ap, char *);
        break;

      case G_TYPE_BOOLEAN:
        // Php::call("var_dump", "boolean");
        internal_parameters[i] = va_arg(ap, gboolean);
        break;

      case G_TYPE_INT:
        // Php::call("var_dump", "int");
        internal_parameters[i] = va_arg(ap, gint);
        break;

      case G_TYPE_UINT:
        // Php::call("var_dump", "int");
        internal_parameters[i] = (int)va_arg(ap, guint);
        break;

      case G_TYPE_OBJECT: {
        // Php::call("var_dump", "object");
        gpointer *e = va_arg(ap, gpointer *);
        internal_parameters[i] = cobject_to_phpobject(e);

        break;
      }
      case G_TYPE_POINTER:
        // Php::call("var_dump", "pointer");
        break;
      case G_TYPE_INTERFACE:
        // Php::call("var_dump", "interface");
        break;
      case G_TYPE_PARAM:
        // Php::call("var_dump", "param");
        break;
      case G_TYPE_BOXED: {
        // ----------------
        // GtkTreePath *e =  va_arg(ap, GtkTreePath *);
        // Php::call("var_dump", gtk_tree_path_to_string(e));
        // internal_parameters[i] = cobject_to_phpobject((gpointer *)e);

        // ----------------
        // gpointer *e = va_arg(ap, gpointer *);

        // Php::call("var_dump", g_type_name(G_TYPE_FROM_CLASS((gpointer *)e)));

        // ----------------
        // GdkEvent *e =  va_arg(ap, GdkEvent *);

        // // Create event from callback
        // GdkEvent_ *event_ = new GdkEvent_();
        // Php::Value gdkevent = Php::Object("GdkEvent", event_);
        // event_->populate(e);

        // internal_parameters[i] = gdkevent;

        break;
      }

      default: {
        // Must not throw: this runs inside GLib's C frames (see the catch
        // below). Report the marshalling gap and pass null for this parameter.
        // g_type_name() returns NULL for an unregistered type.
        const gchar *type_name = g_type_name(callback_object->param_types[i]);
        g_critical("php-gtk3: [generic_callback] unsupported parameter type %s",
                   (type_name != nullptr) ? type_name : "(unknown)");
        internal_parameters[i] = Php::Value();
        break;
      }
    }
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
    Php::call("call_user_func_array", callback_object->callback_name, internal_parameters);
  } catch (Php::Throwable &throwable) {
    callback_error = throwable.what();
    callback_error_code = throwable.code();
    callback_failed = true;
  }

  if (callback_failed) {
    phpgtk_report_callback_exception(callback_error, callback_error_code, callback_object->context);
  }
}