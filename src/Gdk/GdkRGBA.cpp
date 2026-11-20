
#include "GdkRGBA.h"

#include "../../php-gtk.h"

GdkRGBA_::GdkRGBA_() = default;
GdkRGBA_::~GdkRGBA_() = default;

//
GdkRGBA GdkRGBA_::get_instance() {
  return instance;
}

//
void GdkRGBA_::set_instance(GdkRGBA event) {
  instance = event;
}

Php::Value GdkRGBA_::__get(const Php::Value &name) {
  if (name == "red") {
    return instance.red;
  }
  if (name == "green") {
    return instance.green;
  }
  if (name == "blue") {
    return instance.blue;
  }
  if (name == "alpha") {
    return instance.alpha;
  }

  // property not supported, fall back on default
  return Php::Base::__get(name);
}

Php::Value GdkRGBA_::parse(Php::Parameters &parameters) {
  std::string s_spec = parameters[0];
  gchar *spec = (gchar *)s_spec.c_str();

  GdkRGBA rgba;

  bool ret = gdk_rgba_parse(&rgba, spec);
  if (!ret) {
    return ret;
  }

  GdkRGBA_ *return_parsed = new GdkRGBA_();
  return_parsed->set_instance(rgba);
  return Php::Object("GdkRGBA", return_parsed);
}

Php::Value GdkRGBA_::to_string(Php::Parameters &parameters) {
  return phpgtk_take_string(gdk_rgba_to_string(&instance));
}

bool phpgtk_to_rgba(const Php::Value &value, GdkRGBA &color, const char *call) {
  if (value.isString()) {
    std::string spec = value.stringValue();
    if (gdk_rgba_parse(&color, spec.c_str())) {
      return true;
    }
    Php::warning << call << ": cannot parse the color '" << spec << "'" << std::flush;
    return false;
  }
  if (value.isObject() && value.instanceOf("GdkRGBA")) {
    color = ((GdkRGBA_ *)value.implementation())->get_instance();
    return true;
  }
  // what GdkRGBA::parse() returns for a color it cannot read: the same as an unparsable string
  if (value.isBool() && !value.boolValue()) {
    Php::warning << call << ": got false, not a color (a failed GdkRGBA::parse()?)" << std::flush;
    return false;
  }
  throw Php::Exception(std::string(call) + ": expects a GdkRGBA or a color string, got " +
                       phpgtk_type_to_string(value.type()));
}
