# Segfault Fix for WebKit Examples

## Problem
When closing WebKit examples (e.g., `webkit_communication.php`) by clicking the window's close button (X), the application would crash with a segmentation fault.

Additionally, when the segfault was fixed, a new error appeared with enum type parameters in signal callbacks.

## Root Causes

### 1. Segfault During WebKitWebView Destruction
Improper cleanup order of signal handlers connected to the WebKitUserContentManager caused the segfault during destruction.

### 2. Missing Enum and Flags Type Support
The GObject signal callback handler did not support G_TYPE_ENUM and G_TYPE_FLAGS parameters used in GTK/WebKit signals.

## Solution

### Part 1: Fixed WebKitWebView Destructor
- Track all signal handler IDs in a vector
- Explicitly disconnect signals before unreferencing UserContentManager
- Ensures proper cleanup order

### Part 2: Added Enum and Flags Type Support
- Added G_TYPE_ENUM and G_TYPE_FLAGS cases in GObject_::connect_callback
- Both types are passed as integers to PHP callbacks

## Testing
Build with WebKit support and run examples/webkit_communication.php
Verify window opens, signals work, and closes cleanly without errors.
