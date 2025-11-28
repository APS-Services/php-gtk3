
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

// Structure to hold callback data for script message handler
struct ScriptMessageData {
	Php::Callable callback;
	std::string handler_name;
};

// Callback when JavaScript sends a message
static void script_message_received_cb(WebKitUserContentManager *manager, JSCValue *value, gpointer user_data)
{
	ScriptMessageData *data = (ScriptMessageData *)user_data;
	
	// Debug output
	g_print("[DEBUG] Script message received callback triggered for handler: %s\n", data->handler_name.c_str());
	
	// Call the PHP callback
	try {
		// Extract the message value from JavaScript
		if (value != nullptr) {
			// Convert JSCValue to string
			char *str_value = jsc_value_to_string(value);
			
			if (str_value != nullptr) {
				g_print("[DEBUG] Message value: %s\n", str_value);
				
				// Call the PHP callback with the message value
				data->callback(str_value);
				
				g_free(str_value);
			} else {
				g_print("[DEBUG] Message value is null\n");
				// Call with no parameters if value is null
				data->callback();
			}
		} else {
			g_print("[DEBUG] JSCValue is null\n");
			// Call with no parameters if JSCValue is null
			data->callback();
		}
	} catch (const std::exception &e) {
		g_warning("Exception in script message handler callback: %s", e.what());
	} catch (...) {
		g_warning("Unknown exception in script message handler callback");
	}
}

void WebKitWebView_::register_script_message_handler(Php::Parameters &parameters)
{
	if (parameters.size() == 0) {
		throw Php::Exception("register_script_message_handler() expects at least 1 parameter, 0 given");
	}

	std::string s_name = parameters[0];
	const gchar *name = (const gchar *)s_name.c_str();

	// Debug output
	g_print("[DEBUG] Registering script message handler: %s\n", name);

	// Get the user content manager
	WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager(WEBKIT_WEB_VIEW(instance));
	
	if (manager == nullptr) {
		g_warning("[ERROR] User content manager is null!");
		throw Php::Exception("Failed to get user content manager");
	}
	
	g_print("[DEBUG] User content manager obtained: %p\n", (void*)manager);
	
	// Register the script message handler
	webkit_user_content_manager_register_script_message_handler(manager, name);
	
	g_print("[DEBUG] Script message handler registered successfully\n");
	
	// If a callback was provided as second parameter, connect it
	if (parameters.size() > 1 && parameters[1].isCallable()) {
		// Build the signal name: "script-message-received::handlerName"
		std::string signal_name = "script-message-received::" + s_name;
		
		g_print("[DEBUG] Connecting signal: %s\n", signal_name.c_str());
		
		// Create data structure to pass to callback
		ScriptMessageData *data = new ScriptMessageData();
		data->callback = Php::Callable(parameters[1]);
		data->handler_name = s_name;
		
		// Connect the signal to the user content manager (not the webview)
		g_signal_connect_data(manager, signal_name.c_str(), 
		                      G_CALLBACK(script_message_received_cb), 
		                      data, 
		                      [](gpointer user_data, GClosure *closure) {
		                          delete (ScriptMessageData *)user_data;
		                      },
		                      (GConnectFlags)0);
		
		g_print("[DEBUG] Signal connected successfully\n");
	} else {
		g_print("[DEBUG] No callback provided or callback is not callable\n");
	}
}
