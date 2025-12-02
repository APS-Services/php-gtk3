// Platform-specific WebView implementations
// This file includes the appropriate implementation based on the platform

#ifdef _WIN32
    // Windows: Use Microsoft Edge WebView2
    #include "WebKitWebView_Windows.cpp"
#else
    // Unix/Linux/macOS: Use WebKit2GTK
    #include "WebKitWebView_Unix.cpp"
#endif
