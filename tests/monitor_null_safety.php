<?php
/**
 * A GdkMonitor that wraps no monitor must say so, not hand back stack garbage.
 *
 * gdk_display_get_primary_monitor() is documented (nullable), and GDK's Win32
 * backend returns NULL whenever the session enumerated no monitors at all - a
 * disconnected remote desktop session, or a process started without an
 * interactive desktop. The binding wrapped that NULL in a GdkMonitor anyway,
 * so every getter on it tripped a "GDK_IS_MONITOR (monitor) failed" assertion
 * on stderr, and because GDK's out parameter stays untouched after an
 * assertion, get_workarea()/get_geometry() returned whatever the stack held -
 * four ints of garbage that read like a screen size.
 *
 * A GdkMonitor constructed from PHP has the same NULL instance, which is what
 * this test uses: it needs no monitor-less session to reproduce.
 *
 * Usage: php monitor_null_safety.php    (needs a display on X11/Wayland, not on Windows)
 * Expected: exit status 0 and "OK" on stdout - no "BUG:" line, no crash.
 */
// Windows GTK needs no display server; X11/Wayland hosts do, and gtk_init()
// aborts the process rather than failing when it cannot open one
if (PHP_OS_FAMILY !== 'Windows'
    && getenv('DISPLAY') === false
    && getenv('WAYLAND_DISPLAY') === false
) {
    echo "SKIP: Gtk::init() needs a display\n";
    exit(0);
}

Gtk::init();

$failed = false;

// Fill the stack and the allocator with a recognisable pattern, so a getter
// that reports uninitialised memory reports this rather than a plausible 0
$pattern = [];
for ($i = 0; $i < 20000; $i++) {
    $pattern[] = str_repeat(chr(65 + $i % 26), 64);
}
unset($pattern);

/** @var array<string, callable(GdkMonitor): mixed> $getters */
$getters = [
    'get_workarea' => static fn (GdkMonitor $m): mixed => $m->get_workarea(),
    'get_geometry' => static fn (GdkMonitor $m): mixed => $m->get_geometry(),
    'get_width_mm' => static fn (GdkMonitor $m): mixed => $m->get_width_mm(),
    'get_height_mm' => static fn (GdkMonitor $m): mixed => $m->get_height_mm(),
];

$bare = new GdkMonitor();
foreach ($getters as $name => $getter) {
    $value = $getter($bare);

    if ($value !== null) {
        echo "BUG: GdkMonitor::$name() on a monitorless object returned "
            . get_debug_type($value) . ' ' . var_export($value, true) . "\n";
        $failed = true;
    }
}

// The primary monitor is NULL on a display where no monitor is primary, and on
// Windows wherever the session has no monitors: null, or a monitor that answers
// with a real rectangle - never an object that reports garbage.
$primary = GdkDisplay::get_default()->get_primary_monitor();

if ($primary !== null) {
    $workarea = $primary->get_workarea();

    if (!is_array($workarea)) {
        echo 'BUG: the primary monitor reported a work area as '
            . get_debug_type($workarea) . "\n";
        $failed = true;
    } else {
        foreach (['width', 'height'] as $side) {
            // A display GTK can open has a work area; the garbage the assertion
            // used to leave behind was 0x41414141 wide
            if ($workarea[$side] <= 0 || $workarea[$side] > 65535) {
                echo "BUG: the primary monitor reported a work area $side of "
                    . var_export($workarea[$side], true) . "\n";
                $failed = true;
            }
        }
    }
}

if ($failed) {
    exit(1);
}
echo "OK\n";
