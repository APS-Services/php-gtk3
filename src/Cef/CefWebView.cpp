
#include "CefWebView.h"
#include <map>

/**
 * Constructor
 */
CefWebView_::CefWebView_() : 
    current_uri(""), 
    current_title(""), 
    loading(false),
    zoom_level(1.0),
    history_index(-1)
{
}

/**
 * Destructor
 */
CefWebView_::~CefWebView_()
{
    // Clean up resources
    message_handlers.clear();
    history.clear();
}

void CefWebView_::__construct()
{
    // Create a GTK drawing area as the base widget
    // In a full CEF implementation, this would be where the CEF browser is embedded
    instance = (gpointer *)gtk_drawing_area_new();
    
    if (instance == nullptr) {
        throw Php::Exception("Failed to create CefWebView");
    }
    
    // Set default size request for the widget
    gtk_widget_set_size_request(GTK_WIDGET(instance), 800, 600);
    
    // Initialize the widget with a placeholder
    // In a real CEF implementation, this would initialize the CEF browser instance
    current_uri = "about:blank";
    loading = false;
}

void CefWebView_::load_uri(Php::Parameters &parameters)
{
    if (parameters.size() == 0) {
        throw Php::Exception("load_uri() expects at least 1 parameter, 0 given");
    }

    std::string s_uri = parameters[0];
    
    // Add to history
    if (history_index >= 0 && history_index < (int)history.size() - 1) {
        // Remove forward history if we're navigating from middle
        history.erase(history.begin() + history_index + 1, history.end());
    }
    
    history.push_back(s_uri);
    history_index = history.size() - 1;
    
    current_uri = s_uri;
    loading = true;
    
    // In a real CEF implementation, this would call:
    // browser->GetMainFrame()->LoadURL(s_uri);
    
    // Simulate load completion
    loading = false;
}

Php::Value CefWebView_::get_uri()
{
    if (current_uri.empty()) {
        return nullptr;
    }

    return current_uri;
}

void CefWebView_::reload()
{
    if (!current_uri.empty()) {
        loading = true;
        // In a real CEF implementation: browser->Reload();
        loading = false;
    }
}

void CefWebView_::stop_loading()
{
    if (loading) {
        loading = false;
        // In a real CEF implementation: browser->StopLoad();
    }
}

Php::Value CefWebView_::can_go_back()
{
    return history_index > 0;
}

void CefWebView_::go_back()
{
    if (history_index > 0) {
        history_index--;
        current_uri = history[history_index];
        loading = true;
        // In a real CEF implementation: browser->GoBack();
        loading = false;
    }
}

Php::Value CefWebView_::can_go_forward()
{
    return history_index >= 0 && history_index < (int)history.size() - 1;
}

void CefWebView_::go_forward()
{
    if (history_index < (int)history.size() - 1) {
        history_index++;
        current_uri = history[history_index];
        loading = true;
        // In a real CEF implementation: browser->GoForward();
        loading = false;
    }
}

Php::Value CefWebView_::get_title()
{
    if (current_title.empty()) {
        // Return URI as fallback title
        if (!current_uri.empty()) {
            return current_uri;
        }
        return nullptr;
    }

    return current_title;
}

Php::Value CefWebView_::is_loading()
{
    return loading;
}

void CefWebView_::load_html(Php::Parameters &parameters)
{
    if (parameters.size() == 0) {
        throw Php::Exception("load_html() expects at least 1 parameter, 0 given");
    }

    // First parameter: HTML content
    std::string s_content = parameters[0].stringValue();

    // Optional second parameter: base URI
    std::string s_base_uri = "about:blank";
    if (parameters.size() > 1 && !parameters[1].isNull()) {
        s_base_uri = parameters[1].stringValue();
    }

    current_uri = s_base_uri;
    loading = true;
    
    // In a real CEF implementation, this would call:
    // browser->GetMainFrame()->LoadString(s_content, s_base_uri);
    
    loading = false;
}

void CefWebView_::run_javascript(Php::Parameters &parameters)
{
    if (parameters.size() == 0) {
        throw Php::Exception("run_javascript() expects at least 1 parameter, 0 given");
    }

    std::string s_script = parameters[0];
    
    // In a real CEF implementation, this would call:
    // CefRefPtr<CefFrame> frame = browser->GetMainFrame();
    // frame->ExecuteJavaScript(s_script, frame->GetURL(), 0);
}

void CefWebView_::register_script_message_handler(Php::Parameters &parameters)
{
    if (parameters.size() == 0) {
        throw Php::Exception("register_script_message_handler() expects at least 1 parameter, 0 given");
    }

    std::string s_name = parameters[0];
    
    // Store the callback if provided
    if (parameters.size() > 1 && parameters[1].isCallable()) {
        message_handlers[s_name] = parameters[1];
    }
    
    // In a real CEF implementation, this would:
    // 1. Create a CefV8Handler for the named handler
    // 2. Register it with the CefRenderProcessHandler
    // 3. Set up IPC between render and browser processes
}

void CefWebView_::enable_developer_extras()
{
    // In a real CEF implementation, this would enable DevTools:
    // CefWindowInfo windowInfo;
    // CefBrowserSettings settings;
    // browser->GetHost()->ShowDevTools(windowInfo, client, settings, CefPoint());
}

Php::Value CefWebView_::get_zoom_level()
{
    return zoom_level;
}

void CefWebView_::set_zoom_level(Php::Parameters &parameters)
{
    if (parameters.size() == 0) {
        throw Php::Exception("set_zoom_level() expects at least 1 parameter, 0 given");
    }

    double level = parameters[0].floatValue();
    zoom_level = level;
    
    // In a real CEF implementation, this would call:
    // browser->GetHost()->SetZoomLevel(level);
}
