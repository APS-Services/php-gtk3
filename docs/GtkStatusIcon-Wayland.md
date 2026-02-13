# GtkStatusIcon on Wayland - Known Issues and Solutions

## The Problem

When using `GtkStatusIcon` on Wayland-based Linux desktop environments (such as GNOME on Ubuntu with Wayland), you may experience:

1. **Events not being caught**: Click events (`activate`, `popup-menu`, etc.) are not received by your application
2. **Unexpected behavior**: Left-clicking the tray icon may trigger unexpected actions (e.g., opening a browser tab, launching another application)
3. **Icon appears but doesn't work**: The icon may be visible but completely non-interactive

## Why This Happens

### Technical Background

`GtkStatusIcon` is a **deprecated GTK widget** that was designed for X11 window systems. It uses the **XEmbed protocol** which is X11-specific and does not exist in Wayland.

Key facts:
- **GtkStatusIcon is deprecated** since GTK 3.14 (released in 2014)
- **Wayland has no standard system tray protocol** - different desktop environments implement their own solutions
- **X11 compatibility layer limitations**: Even when using XWayland, the tray icon events may not be properly routed

### What Happens on Wayland

When you create a `GtkStatusIcon` on Wayland:

1. The icon may or may not appear (depends on your desktop environment)
2. If it appears, it's often through a compatibility layer that doesn't fully work
3. Click events are not properly routed from the compositor to your application
4. The desktop environment may interpret clicks as generic "activate application" commands
5. This can cause unexpected behavior like launching the wrong application or opening browser tabs

## Solutions and Workarounds

### Solution 0: Install GNOME Extension (For GNOME Users)

**If you're using GNOME 3.26 or newer (Ubuntu 17.10+), even `GDK_BACKEND=x11` won't work without a system tray extension.**

Install one of these GNOME Shell extensions:

1. **AppIndicator and KStatusNotifierItem Support** (recommended):
   ```bash
   # Ubuntu/Debian
   sudo apt install gnome-shell-extension-appindicator
   # Then enable it in GNOME Tweaks or Extensions app
   ```

2. **TopIcons Plus** or **Tray Icons: Reloaded**:
   - Available from https://extensions.gnome.org/
   - Search for "tray icons" or "system tray"

After installing and enabling the extension, restart GNOME Shell (Alt+F2, type 'r', press Enter) or log out and back in.

### Solution 1: Force X11 Backend

Force GTK to use X11 instead of Wayland by setting the `GDK_BACKEND` environment variable:

```bash
GDK_BACKEND=x11 php your_app.php
```

Or in your PHP script:
```php
putenv('GDK_BACKEND=x11');
```

**Pros:**
- Simple and immediate fix
- Works with existing code (if desktop environment supports system tray)
- Full GtkStatusIcon functionality

**Cons:**
- Requires XWayland to be installed
- Not a "native" Wayland solution
- May have minor visual inconsistencies
- **Still requires system tray support in desktop environment (GNOME needs extension)**

### Solution 2: Check Backend at Runtime

Detect Wayland and warn users or disable tray icon functionality:

```php
// Check if running on Wayland
$waylandDisplay = getenv('WAYLAND_DISPLAY');
$gdkBackend = getenv('GDK_BACKEND');

if ($waylandDisplay !== false && $waylandDisplay !== '' && $gdkBackend !== 'x11') {
    echo "Warning: Running on Wayland. System tray icon will not work properly.\n";
    echo "Please restart with: GDK_BACKEND=x11 php your_script.php\n";
    
    // Either exit or disable tray icon functionality
    // exit(1);
}
```

### Solution 3: Use AppIndicator (Long-term Solution)

For native Wayland support, use **AppIndicator** (libayatana-appindicator or libappindicator) instead of GtkStatusIcon.

**Note**: PHP-GTK3 does not currently include AppIndicator bindings. This would require:
- Adding bindings for libayatana-appindicator3
- Or using an external library/wrapper

### Solution 4: Detect and Adapt

Provide different UX based on the platform:

```php
function is_wayland_without_x11_backend() {
    $waylandDisplay = getenv('WAYLAND_DISPLAY');
    $gdkBackend = getenv('GDK_BACKEND');
    
    return ($waylandDisplay !== false && $waylandDisplay !== '' && $gdkBackend !== 'x11');
}

if (is_wayland_without_x11_backend()) {
    // Don't create tray icon, use alternative UI
    // For example: show a regular window with minimize-to-tray option disabled
    echo "System tray not available. Using window-based UI.\n";
} else {
    // Create tray icon normally
    $statusIcon = new GtkStatusIcon();
    // ... configure icon ...
}
```

## Example: Proper GtkStatusIcon Usage

See `examples/statusicon_example.php` for a complete working example that:
- Detects Wayland and warns the user
- Shows how to connect signals properly
- Demonstrates both left-click (`activate`) and right-click (`popup-menu`) handling
- Includes proper menu positioning

## Desktop Environment Compatibility

**IMPORTANT: GNOME has removed system tray support** starting with GNOME 3.26 (2017). Even on X11, GtkStatusIcon will not work on modern GNOME without extensions.

| Desktop Environment | Wayland | X11 | Notes |
|---------------------|---------|-----|-------|
| GNOME 3.26+ (Ubuntu 17.10+) | ❌ No | ❌ No | System tray removed. Need TopIcons or AppIndicator extension |
| GNOME 3.24 and older | ❌ No | ✅ Yes | Use GDK_BACKEND=x11 |
| KDE Plasma | ⚠️ Limited | ✅ Yes | SNI support, may work with compatibility bridge |
| XFCE | N/A | ✅ Yes | X11 only, full support |
| MATE | N/A | ✅ Yes | X11 only, full support |
| Cinnamon | ⚠️ Limited | ✅ Yes | Some Wayland support |
| Ubuntu Unity | N/A | ✅ Yes | X11 only, full support |

## Testing Your Application

### On X11:
```bash
php your_app.php
```

### On Wayland with X11 backend:
```bash
GDK_BACKEND=x11 php your_app.php
```

### Check if icon is embedded:
```php
$icon = new GtkStatusIcon();
$icon->set_from_icon_name("applications-system");
$icon->set_visible(true);

if ($icon->is_embedded()) {
    echo "Icon successfully embedded in system tray\n";
} else {
    echo "WARNING: Icon NOT embedded - no system tray available\n";
}
```

## Frequently Asked Questions

### Q: Why does left-click open a browser tab even with GDK_BACKEND=x11?

This happens on **GNOME 3.26+** (Ubuntu 17.10+, Ubuntu 18.04+, Ubuntu 20.04+, etc.) because:

1. **GNOME removed system tray support entirely** in version 3.26 (September 2017)
2. Without a system tray, the icon cannot be displayed
3. GTK may try to "activate" the application instead, leading to unexpected behavior
4. If your PHP process has any association with Chrome/Chromium, it might open that instead

**Solutions:**
1. **Install a GNOME extension** for system tray support (see "Solution 0" above)
2. **Use a different desktop environment** (XFCE, MATE, KDE)
3. **Switch to AppIndicator** (requires adding bindings to PHP-GTK3)

### Q: How do I know if my system has a system tray?

Check if the icon is embedded:
```php
$icon = new GtkStatusIcon();
$icon->set_from_icon_name("applications-system");
$icon->set_visible(true);

if (!$icon->is_embedded()) {
    echo "ERROR: No system tray available!\n";
    echo "On GNOME, install gnome-shell-extension-appindicator\n";
    exit(1);
}
```

### Q: Why does left-click open a browser tab on Wayland?

This happens when:
1. Your application is running on Wayland
2. The desktop environment can't properly handle the tray icon events
3. The DE falls back to "activate application" behavior
4. Your application process is somehow associated with Chrome/Chromium (e.g., if launched from a Chrome-based IDE or terminal)

**Solution**: Use `GDK_BACKEND=x11` AND install a system tray extension if using GNOME.

### Q: Can I make it work natively on Wayland?

No, not with `GtkStatusIcon`. You would need to use AppIndicator or StatusNotifierItem protocols, which are not currently bound in PHP-GTK3.

### Q: Will this be fixed in the future?

`GtkStatusIcon` is deprecated by GTK and will never work properly on Wayland. The proper fix is to add AppIndicator support to PHP-GTK3, but this requires significant development work.

### Q: How do I detect if I'm on Wayland?

```php
$waylandDisplay = getenv('WAYLAND_DISPLAY');
if ($waylandDisplay !== false && $waylandDisplay !== '') {
    echo "Running on Wayland\n";
} else {
    echo "Running on X11 or other\n";
}
```

### Q: What signals does GtkStatusIcon support?

On X11, these signals work:
- `activate` - Left-click
- `popup-menu` - Right-click
- `button-press-event` - Button press with event details
- `button-release-event` - Button release
- `scroll-event` - Mouse scroll
- `size-changed` - Icon size changed

On Wayland, **none of these signals work reliably**.

## References

- [GTK 3 GtkStatusIcon Documentation](https://docs.gtk.org/gtk3/class.StatusIcon.html)
- [GTK Migration Guide - GtkStatusIcon](https://docs.gtk.org/gtk4/migrating-3to4.html#stop-using-gtkstatusicon)
- [Wayland System Tray Specification](https://freedesktop.org/wiki/Specifications/StatusNotifierItem/)
- [AppIndicator Project](https://launchpad.net/libappindicator)

## Summary

**For immediate use:** Set `GDK_BACKEND=x11` to force X11 mode and get full functionality.

**For production applications on Wayland:** Consider alternative UI patterns that don't rely on system tray, or contribute AppIndicator bindings to PHP-GTK3.
