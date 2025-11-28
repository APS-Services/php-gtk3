
#include "WebKitWebView.h"

/**
 * Constructor
 */
WebKitWebView_::WebKitWebView_() = default;

/**
 * Destructor
 */
WebKitWebView_::~WebKitWebView_() = default;

void WebKitWebView_::__construct()
{
	instance = (gpointer *)webkit_web_view_new();
}

void WebKitWebView_::load_uri(Php::Parameters &parameters)
{
	if (parameters.size() == 0) {
		throw Php::Exception("load_uri() expects at least 1 parameter, 0 given");
	}

	std::string s_uri = parameters[0];
	const gchar *uri = (const gchar *)s_uri.c_str();

	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(instance), uri);
}

Php::Value WebKitWebView_::get_uri()
{
	const gchar *ret = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(instance));

	if (ret == nullptr) {
		return nullptr;
	}

	return std::string(ret);
}

void WebKitWebView_::reload()
{
	webkit_web_view_reload(WEBKIT_WEB_VIEW(instance));
}

void WebKitWebView_::stop_loading()
{
	webkit_web_view_stop_loading(WEBKIT_WEB_VIEW(instance));
}

Php::Value WebKitWebView_::can_go_back()
{
	gboolean ret = webkit_web_view_can_go_back(WEBKIT_WEB_VIEW(instance));

	return (bool)ret;
}

void WebKitWebView_::go_back()
{
	webkit_web_view_go_back(WEBKIT_WEB_VIEW(instance));
}

Php::Value WebKitWebView_::can_go_forward()
{
	gboolean ret = webkit_web_view_can_go_forward(WEBKIT_WEB_VIEW(instance));

	return (bool)ret;
}

void WebKitWebView_::go_forward()
{
	webkit_web_view_go_forward(WEBKIT_WEB_VIEW(instance));
}

Php::Value WebKitWebView_::get_title()
{
	const gchar *ret = webkit_web_view_get_title(WEBKIT_WEB_VIEW(instance));

	if (ret == nullptr) {
		return nullptr;
	}

	return std::string(ret);
}

Php::Value WebKitWebView_::is_loading()
{
	gboolean ret = webkit_web_view_is_loading(WEBKIT_WEB_VIEW(instance));

	return (bool)ret;
}

void WebKitWebView_::load_html(Php::Parameters &parameters)
{
    if (parameters.size() == 0) {
        throw Php::Exception("load_html() expects at least 1 parameter, 0 given");
    }

    // First parameter: HTML content
    std::string s_content = parameters[0].stringValue();
    const gchar *content = s_content.c_str();

    // Optional second parameter: base URI
    std::string s_base_uri;
    const gchar *base_uri = nullptr;

    if (parameters.size() > 1 && !parameters[1].isNull()) {
        s_base_uri = parameters[1].stringValue();
        base_uri = s_base_uri.c_str();
    }

    webkit_web_view_load_html(WEBKIT_WEB_VIEW(instance), content, base_uri);
}

void WebKitWebView_::run_javascript(Php::Parameters &parameters)
{
	if (parameters.size() == 0) {
		throw Php::Exception("run_javascript() expects at least 1 parameter, 0 given");
	}

	std::string s_script = parameters[0];
	const gchar *script = (const gchar *)s_script.c_str();

	webkit_web_view_evaluate_javascript(WEBKIT_WEB_VIEW(instance), script, -1, nullptr, nullptr, nullptr, nullptr, nullptr);
}

void WebKitWebView_::register_script_message_handler(Php::Parameters &parameters)
{
	if (parameters.size() == 0) {
		throw Php::Exception("register_script_message_handler() expects at least 1 parameter, 0 given");
	}

	std::string s_name = parameters[0];
	const gchar *name = (const gchar *)s_name.c_str();

	WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager(WEBKIT_WEB_VIEW(instance));
	webkit_user_content_manager_register_script_message_handler(manager, name);
}
