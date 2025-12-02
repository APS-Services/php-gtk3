# Cross-Platform WebView Support

## Overview

This document addresses the question of using the [webview library](https://github.com/webview/webview) for cross-platform WebView support in PHP-GTK3.

## Current WebView Implementation

PHP-GTK3 currently supports WebView widgets through **WebKitWebView** (when compiled with `WITH_WEBKIT=1`), which provides:

- **Platform**: Linux-focused (uses WebKit2GTK)
- **Integration**: Full GTK widget that can be embedded in any GTK container
- **Features**: Complete WebKit2GTK API including JavaScript bidirectional communication
- **Use Case**: Embedding web content within GTK applications

See [webkit.md](webkit.md) for detailed documentation.

## The webview Library

The [webview library](https://github.com/webview/webview) is a tiny cross-platform library that provides:

### Platform Support

- **Linux**: GTK + WebKitGTK (same backend as current implementation)
- **macOS**: Cocoa + WebKit
- **Windows**: Windows API + WebView2 (Edge/Chromium)

### Architecture Differences

The webview library is fundamentally different from PHP-GTK3's WebKitWebView:

| Feature | PHP-GTK3 WebKitWebView | webview Library |
|---------|------------------------|-----------------|
| Platform | Linux (WebKit2GTK) | Linux, macOS, Windows |
| Integration | GTK widget (embeddable) | Standalone window |
| Window Management | Part of GTK window hierarchy | Creates its own window |
| Widget Support | Can be added to any GtkContainer | Cannot be embedded as a widget |
| API Style | GTK/WebKit2GTK native API | Simplified cross-platform API |

## Key Architectural Incompatibility

The **webview library creates its own native window** and is **not designed to be embedded as a GTK widget**. This means:

1. ❌ Cannot use `$container->add($webview)` - not a GTK widget
2. ❌ Cannot integrate with existing GTK window layouts
3. ❌ Cannot use GTK sizing, positioning, or container features
4. ✅ Can create standalone webview windows
5. ✅ Provides cross-platform abstraction

## Recommended Approaches

### For Linux-Only Applications with GTK Integration

**Use the current WebKitWebView implementation:**

```php
<?php
Gtk::init();

$window = new GtkWindow();
$window->set_default_size(800, 600);

// WebKitWebView is a GTK widget - fully integrated
$webView = new WebKitWebView();
$scrolled = new GtkScrolledWindow();
$scrolled->add($webView);
$window->add($scrolled);

$webView->load_uri("https://www.example.com");

$window->show_all();
Gtk::main();
```

**Advantages:**
- Full GTK integration
- Can combine with other GTK widgets
- Native WebKit2GTK API
- Bidirectional JavaScript ↔ PHP communication
- Proven implementation

### For Cross-Platform Standalone WebView Windows

If you need cross-platform support (Windows/macOS/Linux) and don't need GTK widget integration, you could:

1. **Use a separate webview process**
   - Launch the webview library as a separate executable
   - Communicate via IPC (Inter-Process Communication)
   - Not ideal for tight integration

2. **Create a standalone WebView widget wrapper (complex)**
   - Would require creating a new PHP-GTK3 class that:
     - Creates a standalone window using webview library
     - Manages that window independently
     - Provides a simplified API
   - **Limitation**: Cannot be embedded in GTK containers
   - **Complexity**: Requires platform-specific compilation
   - **Maintenance**: Must keep up with webview library updates

### For True Cross-Platform GTK Applications

If you need your entire application to work cross-platform, consider:

1. **Compile PHP-GTK3 on each platform**
   - Linux: Use WebKitWebView (WebKit2GTK)
   - macOS: Use WebKitWebView (GTK can be compiled on macOS with WebKitGTK)
   - Windows: GTK3 can run on Windows with WebKitGTK (though less common)

2. **Platform-specific UI backends**
   - Use different UI frameworks per platform
   - Not ideal, but sometimes necessary for best native integration

## Implementation Considerations

### If Standalone WebView Support is Added

If there's strong interest in adding standalone webview library support, it would look like:

```php
<?php
// This would NOT be a GTK widget
$standaloneWebView = new WebView();
$standaloneWebView->set_title("Standalone WebView");
$standaloneWebView->set_size(800, 600);
$standaloneWebView->set_html("<h1>Hello World</h1>");
$standaloneWebView->run(); // Blocks until window closes
```

**Key Limitations:**
- Cannot use with GTK containers
- Cannot combine with other GTK widgets
- Creates separate window outside GTK hierarchy
- Different event loop (conflicts with Gtk::main())

### Technical Challenges

1. **Event Loop Conflicts**: The webview library has its own event loop that would conflict with GTK's event loop
2. **Memory Management**: Different memory management between GTK and webview library
3. **Platform Dependencies**: Requires platform-specific compilation and dependencies
4. **Maintenance Burden**: Another library to maintain and keep updated

## Conclusion

### Current Recommendation

For PHP-GTK3 applications, **continue using the existing WebKitWebView** implementation because:

1. ✅ Full GTK widget integration
2. ✅ Can be embedded in any GTK container
3. ✅ Works with GTK layout and sizing
4. ✅ Mature and stable implementation
5. ✅ Complete WebKit2GTK API

### When to Consider webview Library

The webview library would only be beneficial if:

1. You need **standalone** webview windows (not embedded widgets)
2. You need **Windows support** specifically
3. You don't need GTK widget integration
4. You're willing to manage platform-specific builds

### Future Possibilities

If there's significant demand for Windows/macOS support with embedded webviews:

1. **Better approach**: Port GTK3/WebKitGTK to Windows more fully
2. **Alternative**: Create platform-specific implementations:
   - Linux: WebKitWebView (current)
   - Windows: Custom widget using WebView2
   - macOS: Custom widget using WebKit
3. This would maintain widget compatibility while supporting multiple platforms

## Contributing

If you're interested in adding cross-platform webview support, please:

1. Open a discussion issue to outline the use case
2. Discuss architectural approaches
3. Consider the event loop and widget integration challenges
4. Propose an API that maintains consistency with GTK patterns

For questions or suggestions, please visit the [issue tracker](https://github.com/scorninpc/php-gtk3/issues).
