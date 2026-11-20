<?php
/**
 * Strings GTK returns must reach PHP intact, null where GTK has none, and be
 * freed where GTK hands them over; color arguments must be checked.
 *
 * The getters copied GTK's string into a std::string: a NULL (no tooltip, no
 * active combo entry, an empty GtkStack) aborted the process, and a string GTK
 * hands over (GdkRGBA::to_string(), get_selected_rows() paths, text buffer
 * text) was never freed - a POS that colors its buttons through to_string()
 * grew all day. override_background_color() used uninitialised memory for a
 * color it could not parse, and crashed or read garbage for anything but a
 * string or a GdkRGBA.
 *
 * Usage: php string_returns.php    (needs a display on X11/Wayland, not on Windows)
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
$expect = function (string $what, $actual, $expected) use (&$failed): void {
    if ($actual !== $expected) {
        echo "BUG: $what gave " . var_export($actual, true) . ', expected ' . var_export($expected, true) . "\n";
        $failed = true;
    }
};

// GTK has no string: null, not an abort
$expect('get_tooltip_text() without a tooltip', (new GtkButton())->get_tooltip_text(), null);
$expect('get_active_text() with nothing active', (new GtkComboBoxText())->get_active_text(), null);
$expect('get_visible_child_name() of an empty stack', (new GtkStack())->get_visible_child_name(), null);
$expect('get_label() of a button without one', (new GtkButton())->get_label(), null);

// GTK has a string: it arrives intact, and one GTK keeps is not freed by reading it
$button = new GtkButton();
$button->set_tooltip_text('tip');
$expect('get_tooltip_text()', $button->get_tooltip_text(), 'tip');
$expect('GdkRGBA::to_string()', GdkRGBA::parse('#ff0000')->to_string(), 'rgb(255,0,0)');
$frame = new GtkFrame('lbl');
$expect('get_label() twice', [$frame->get_label(), $frame->get_label()], ['lbl', 'lbl']);

$store = new GtkListStore(GObject::TYPE_STRING);
$first = $store->append(['a']);
$store->append(['b']);
$view = new GtkTreeView();
$view->set_model($store);
$selection = $view->get_selection();
$selection->set_mode(GtkSelectionMode::MULTIPLE);
$expect('get_selected_rows() with nothing selected', $selection->get_selected_rows()[1], []);
$selection->select_all();
$expect('get_selected_rows() paths', $selection->get_selected_rows()[1], ['0', '1']);
$expect('get_path()', $store->get_path($first), '0');
$long = $store->append([str_repeat('x', 200)]);
$expect('get_value()', $store->get_value($long, 0), str_repeat('x', 200));

// colors: a GdkRGBA and a string work, an unparsable string warns, anything else throws
$warnings = [];
set_error_handler(function (int $errno, string $message) use (&$warnings): bool {
    $warnings[] = $message;
    return true;
});
$button->override_background_color(GtkStateFlags::NORMAL, GdkRGBA::parse('#00ff00'));
$button->override_background_color(GtkStateFlags::NORMAL, '#00ff00');
$expect('warnings for valid colors', $warnings, []);
$color = fn (): string => $button->get_style_context()->get_color(GtkStateFlags::NORMAL)->to_string();
$button->override_color(GtkStateFlags::NORMAL, '#00ff00');
$button->override_color(GtkStateFlags::NORMAL, 'no-such-color');
$expect('warnings for an unparsable color', count($warnings), 1);
$expect('the warning names the binding', strpos($warnings[0] ?? '', 'override_color') !== false, true);
$expect('the color after an unparsable one', $color(), 'rgb(0,255,0)');
// what GdkRGBA::parse() returns for a color it cannot read
$button->override_color(GtkStateFlags::NORMAL, false);
$expect('warnings for false', count($warnings), 2);
$button->override_color(GtkStateFlags::NORMAL, null);
$expect('null undoing the override', $color() !== 'rgb(0,255,0)', true);
// a color button shows whether an unparsable color left the color as it was
$colorButton = new GtkColorButton();
$colorButton->set_rgba('#ff0000');
$colorButton->set_rgba('no-such-color');
$expect('set_rgba() after an unparsable color', $colorButton->get_rgba()->to_string(), 'rgb(255,0,0)');
$expect('new_with_rgba() with an unparsable color', GtkColorButton::new_with_rgba('nope')->get_rgba()->to_string(), 'rgb(0,0,0)');
$expect('warnings for the color button', count($warnings), 4);
restore_error_handler();
try {
    $button->override_background_color(GtkStateFlags::NORMAL, 42);
    echo "BUG: override_background_color() accepted an int\n";
    $failed = true;
} catch (Exception $e) {
}

// strings GTK hands over are freed: the process does not grow with the calls. The peak
// (KB, on Linux and on Windows) grows only while memory is kept, which is what a leak does.
$peak = fn (): int => getrusage()['ru_maxrss'] * 1024;
$rgba = GdkRGBA::parse('#123456');
// each on its own: the heap refills freed holes first, so one big leak would hide a small one
$calls = [
    'to_string()' => [fn () => $rgba->to_string(), 1000000],
    'get_selected_rows()' => [fn () => $selection->get_selected_rows(), 200000],
    'get_value()' => [fn () => $store->get_value($long, 0), 200000],
];
$measure = function (callable $call, int $rounds) use ($peak): int {
    for ($i = 0; $i < 1000; $i++) {
        $call();
    }
    $before = $peak();
    for ($i = 0; $i < $rounds; $i++) {
        $call();
    }
    return $peak() - $before;
};
// the calls themselves, with no string GTK hands over: php.exe on Windows CI grew by some 11 MB
// per 1000000 of them, so only what a check keeps on top of that counts
$label = new GtkLabel('kept by GTK');
$base = $measure(fn () => $label->get_text(), 1000000);
echo 'INFO: 1000000 calls without an owned string kept ' . round($base / 1048576, 1) . " MB\n";
foreach ($calls as $what => [$call, $rounds]) {
    $grown = $measure($call, $rounds) - $base * $rounds / 1000000;
    // leaking, each keeps some 30-50 MB on top; freed, about nothing
    if ($grown > 4 * 1024 * 1024) {
        echo "BUG: $rounds $what calls kept " . round($grown / 1048576, 1) . " MB more than calls without an owned string\n";
        $failed = true;
    }
}

if ($failed) {
    exit(1);
}
echo "OK\n";
