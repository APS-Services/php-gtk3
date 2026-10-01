<?php
/**
 * A URI that cannot be opened must be reported, not swallowed.
 *
 * Gtk::show_uri_on_window() declared its out-parameter as an uninitialised
 * `GError **error;` - marked `// @TODO` - and passed it straight to
 * gtk_show_uri_on_window(). On failure GLib dereferences that pointer to see
 * whether an error is already set, so it read through whatever the stack
 * happened to hold: either a wild read, or - as seen on this host - GLib
 * noticing and refusing to write:
 *
 *   GLib-WARNING **: GError set over the top of a previous GError or
 *   uninitialized memory. This indicates a bug in someone's code.
 *
 * Either way the reason never reached PHP: the caller got false and no
 * message. The `guint32 timestamp;` next to it was uninitialised too, so the
 * event time handed to GTK was whatever was on the stack.
 *
 * Usage: php show_uri_error.php      (needs a display on X11/Wayland, not on Windows)
 * Expected: exit status 0 and "OK" on stdout - no "BUG:" line, no crash and
 *           no GLib-WARNING about uninitialized memory.
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

/**
 * Asks GTK to open a URI no handler can take.
 *
 * @param array<int, mixed> $arguments
 *
 * @return string The reported reason, or '' when nothing was reported
 */
function showUnhandledUri(array $arguments): string
{
    try {
        $returned = Gtk::show_uri_on_window(...$arguments);
        echo "BUG: an unhandled URI returned " . var_export($returned, true)
            . " instead of reporting the reason\n";

        return '';
    } catch (Exception $e) {
        return $e->getMessage();
    }
}

$uri = 'nonexistent-scheme-xyz://nowhere';
$probes = [
    'without a timestamp' => [null, $uri],
    'with a timestamp'    => [null, $uri, 0],
];
foreach ($probes as $label => $arguments) {
    $message = showUnhandledUri($arguments);
    if ($message === '') {
        $failed = true;
        continue;
    }
    if (!str_contains($message, 'Failed to show the URI')) {
        echo "BUG: $label: the reason was reported as \"$message\"\n";
        $failed = true;
    }
}

// Reuses the stack the uninitialised pointer was read from, and makes the
// allocator work, so damage shows up here rather than at some later exit
$blocks = [];
for ($i = 0; $i < 20000; $i++) {
    $blocks[] = str_repeat(chr(65 + $i % 26), 64);
}

if ($failed) {
    exit(1);
}
echo "OK\n";
