<?php
/**
 * A GLib call that fails must say why, and an out-parameter must come back.
 *
 * Thirteen bindings created a GError, handed it to GLib and then dropped it:
 * the message leaked and the caller got a bare false. Five of them
 * (GtkPrintSettings and GtkPageSetup load_file/to_file/new_from_file) declared
 * it uninitialised, so GLib read through whatever the stack held when it
 * checked whether an error was already set. The two new_from_file() bindings
 * also wrapped a NULL on failure, which crashed on the first method call
 * instead of at the failure.
 *
 * GtkAlignment::get_padding() was a different shape of the same mistake: four
 * uninitialised guint* passed to GTK by value, so GTK wrote the padding through
 * wild pointers, and the returned array held the addresses of those pointers
 * rather than the padding.
 *
 * Usage: php glib_error_reporting.php    (needs a display on X11/Wayland, not on Windows)
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

/** @var array<int, string> $warnings */
$warnings = [];
set_error_handler(function (int $number, string $message) use (&$warnings): bool {
    $warnings[] = $message;

    return true;
});

/**
 * Runs $probe and returns the warnings it produced.
 *
 * @param callable $probe
 *
 * @return array<int, string>
 */
function warningsFrom(callable $probe): array
{
    global $warnings;

    $warnings = [];
    $probe();

    return $warnings;
}

// A stylesheet GTK cannot parse: the call still answers false, but the reason
// must come through instead of being dropped
$provider = new GtkCssProvider();
$returned = null;
$reported = warningsFrom(function () use ($provider, &$returned): void {
    $returned = $provider->load_from_data('@@@ this is not css {{{ ;;;');
});

if ($returned !== false) {
    echo "BUG: broken css returned " . var_export($returned, true) . ", expected false\n";
    $failed = true;
}
if (count($reported) !== 1 || !str_contains($reported[0], 'GtkCssProvider::load_from_data')) {
    echo "BUG: broken css reported " . var_export($reported, true) . "\n";
    $failed = true;
}

// Valid css must stay quiet
$reported = warningsFrom(function () use ($provider, &$returned): void {
    $returned = $provider->load_from_data('window { color: red; }');
});
if ($returned !== true || $reported !== []) {
    echo "BUG: valid css returned " . var_export($returned, true)
        . " and reported " . var_export($reported, true) . "\n";
    $failed = true;
}

// The same for a description GtkBuilder cannot read
$builder = new GtkBuilder();
$reported = warningsFrom(function () use ($builder): void {
    $builder->add_from_string('<interface><object class="NoSuchWidgetClass"/></interface>');
});
if (count($reported) !== 1 || !str_contains($reported[0], 'GtkBuilder::add_from_string')) {
    echo "BUG: a broken builder description reported " . var_export($reported, true) . "\n";
    $failed = true;
}

// Padding comes back as the numbers that were set, not as stack addresses
$alignment = new GtkAlignment(0, 0, 1, 1);
$alignment->set_padding(3, 5, 7, 11);
$expected = ['top' => 3, 'bottom' => 5, 'left' => 7, 'right' => 11];
$padding = $alignment->get_padding();
if ($padding !== $expected) {
    echo "BUG: the padding came back as " . var_export($padding, true) . "\n";
    $failed = true;
}

// A settings file that cannot be read must raise, not hand back an object
// wrapping NULL. GtkPageSetup registers new_from_file() as static and
// GtkPrintSettings as an instance method, so each is called as declared.
$missing = '/nonexistent/definitely-not-here.ini';
$probes = [
    'GtkPageSetup' => static fn (): mixed => GtkPageSetup::new_from_file($missing),
    'GtkPrintSettings' => static fn (): mixed => (new GtkPrintSettings())->new_from_file($missing),
];
foreach ($probes as $class => $probe) {
    try {
        $object = $probe();
        echo "BUG: $class::new_from_file() handed back "
            . get_debug_type($object) . " for a missing file\n";
        $failed = true;
    } catch (Exception $e) {
        if (!str_contains($e->getMessage(), $class . '::new_from_file')) {
            echo "BUG: $class::new_from_file() raised \"" . $e->getMessage() . "\"\n";
            $failed = true;
        }
    }
}

restore_error_handler();

// Make the allocator work with whatever was freed or written over
$blocks = [];
for ($i = 0; $i < 20000; $i++) {
    $blocks[] = str_repeat(chr(65 + $i % 26), 64);
}

if ($failed) {
    exit(1);
}
echo "OK\n";
