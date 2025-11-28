# WebKit Event Dispatching Fix

## Problem
When running `php8.4 examples/webkit_js_to_php.php`, button clicks on the test page were not triggering events. Messages sent from JavaScript using `window.webkit.messageHandlers.phpApp.postMessage()` were not being received by the PHP callback.

## Root Causes

### 1. Incorrect Callback Signature
The callback signature for the `script-message-received` signal was incorrect. The implementation was using `WebKitJavascriptResult*` as the second parameter, but WebKit2GTK's `script-message-received::handlerName` signal actually passes a `JSCValue*` parameter.

### 2. Improper WebView Initialization
More critically, WebKit2GTK requires a specific initialization pattern for script message handlers to work:
- The `UserContentManager` must be created **first**
- Script message handlers must be registered on that manager
- The `WebView` must be created **with** that manager using `webkit_web_view_new_with_user_content_manager()`

The previous approach of creating the WebView first and then getting its manager didn't work properly for script message handlers.

## Solution

### 1. Fixed Callback Signature (C++)
Changed the callback in `src/WebKit/WebKitWebView.cpp`:

**Before:**
```cpp
static void script_message_received_cb(WebKitUserContentManager *manager, 
                                       WebKitJavascriptResult *js_result, 
                                       gpointer user_data)
```

**After:**
```cpp
static void script_message_received_cb(WebKitUserContentManager *manager, 
                                       JSCValue *value, 
                                       gpointer user_data)
```

### 2. Proper WebView Initialization Pattern
Changed the initialization in `src/WebKit/WebKitWebView.cpp`:

**Before:**
```cpp
void WebKitWebView_::__construct()
{
    instance = (gpointer *)webkit_web_view_new();
}

void register_script_message_handler(...)
{
    WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager(WEBKIT_WEB_VIEW(instance));
    webkit_user_content_manager_register_script_message_handler(manager, name);
}
```

**After:**
```cpp
// In header: private member
WebKitUserContentManager *user_content_manager;

void WebKitWebView_::__construct()
{
    user_content_manager = webkit_user_content_manager_new();
    instance = (gpointer *)webkit_web_view_new_with_user_content_manager(user_content_manager);
}

void register_script_message_handler(...)
{
    webkit_user_content_manager_register_script_message_handler(user_content_manager, name);
}
```

### 3. Added Message Extraction
Implemented proper message extraction using JavaScriptCore functions:
- Added `jsc_value_to_string()` to convert JavaScript values to C strings
- Passes the extracted message data to the PHP callback
- Handles null values gracefully

### 4. Added Required Header
Added `#include <jsc/jsc.h>` to `src/WebKit/WebKitWebView.h` for JavaScriptCore types.

### 5. Enhanced PHP Callback
Updated the PHP callback in `examples/webkit_js_to_php.php` to:
- Accept the message data parameter
- Display the actual message content
- Show comprehensive debug output

### 6. Proper Memory Management
Added proper cleanup in destructor:
- Unreferences the UserContentManager when the WebKitWebView is destroyed
- Prevents memory leaks

### 7. Added Debug Output
Added extensive debug logging to help troubleshoot:
- C++ logs: handler registration, signal connection, message reception
- PHP logs: application lifecycle events, callback invocations

## Debug Output
When running the example, you should see output like:

```
[PHP] Starting JavaScript → PHP Messaging Example
[PHP] ================================================
[PHP] Creating WebKitWebView widget...
[PHP] WebKitWebView created successfully
[PHP] Registering script message handler 'phpApp'...
[DEBUG] Registering script message handler: phpApp
[DEBUG] User content manager obtained: 0x...
[DEBUG] Script message handler registered successfully
[DEBUG] Connecting signal: script-message-received::phpApp
[DEBUG] Signal connected successfully
[PHP] Script message handler registered successfully
...
[DEBUG] Script message received callback triggered for handler: phpApp
[DEBUG] Message value: Hello from JavaScript!
[PHP DEBUG] Callback invoked with data: 'Hello from JavaScript!'
```

## Testing
To test the fix:

1. Build the extension with WebKit support:
   ```bash
   make WITH_WEBKIT=1 -j 4
   sudo make install
   ```

2. Run the example:
   ```bash
   php -dextension=./php-gtk3.so examples/webkit_js_to_php.php
   ```

3. Click the buttons in the UI:
   - "Send Simple Message"
   - "Send Text Data"
   - "Send Object Data"
   - "Send Counter Value"

4. Verify:
   - Debug output appears in the terminal
   - Messages appear in the text log area in the UI
   - The automatic "Page loaded successfully!" message appears on page load

## Technical Details

### WebKit2GTK Signal Documentation
The `script-message-received::name` signal is emitted when JavaScript code calls:
```javascript
window.webkit.messageHandlers.name.postMessage(data)
```

Signal signature:
```c
void user_function (WebKitUserContentManager *manager,
                    JSCValue *value,
                    gpointer user_data)
```

### JavaScriptCore API
- `jsc_value_to_string()` - Converts a JSCValue to a C string
- Must use `g_free()` to free the returned string
- Returns NULL if the value cannot be converted

## Files Modified
1. `src/WebKit/WebKitWebView.h` - Added JSC header
2. `src/WebKit/WebKitWebView.cpp` - Fixed callback signature and added message extraction
3. `examples/webkit_js_to_php.php` - Enhanced callback and added debug output

## References
- [WebKit2GTK API Documentation](https://webkitgtk.org/reference/webkit2gtk/stable/)
- [JavaScriptCore API Documentation](https://webkitgtk.org/reference/jsc-glib/stable/)
