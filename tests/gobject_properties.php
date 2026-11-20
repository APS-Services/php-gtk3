<?php
/**
 * GObject::get_property() and set_property() must work for every common
 * property type, not only objects.
 *
 * get_property() asked GLib for every property as an object and read the
 * result back by the wrong macro, so it threw "G_TYPE_INVALID not
 * implemented" for an int, a string or a boolean alike. set_property()
 * refused enum, flags and unsigned properties ('justify', 'input-hints',
 * 'border-width').
 *
 * Usage: php gobject_properties.php    (needs a display on X11/Wayland, not on Windows)
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
$roundTrip = function (GObject $object, string $property, $value, $expected = null) use (&$failed): void {
    $expected = func_num_args() > 3 ? $expected : $value;
    try {
        $object->set_property($property, $value);
        $actual = $object->get_property($property);
    } catch (Exception $e) {
        $actual = 'exception: ' . $e->getMessage();
    }
    if ($actual !== $expected) {
        echo 'BUG: ' . get_class($object) . "::$property gave " . var_export($actual, true)
            . ', expected ' . var_export($expected, true) . "\n";
        $failed = true;
    }
};

$label = new GtkLabel('x');
$roundTrip($label, 'width-chars', 12);          // int
$roundTrip($label, 'justify', 2);               // enum (GTK_JUSTIFY_CENTER)
$roundTrip($label, 'xalign', 0.25);             // float
$roundTrip(new GtkProgressBar(), 'fraction', 0.25); // double
$roundTrip(new GtkEntry(), 'input-hints', 2);   // flags (GTK_INPUT_HINT_SPELLCHECK)
$button = new GtkButton();
$roundTrip($button, 'label', 'text');           // string
$roundTrip($button, 'tooltip-text', null);      // string, unset
$roundTrip($button, 'sensitive', false);        // boolean
$roundTrip($button, 'border-width', 5);         // guint

$view = new GtkTreeView();
$store = new GtkListStore(GObject::TYPE_STRING);
$view->set_model($store);
$model = $view->get_property('model');
if (!$model instanceof GtkListStore) {
    echo "BUG: GtkTreeView::model is no GtkListStore\n";
    $failed = true;
} else {
    // the returned model must work as a store, not only carry the class name
    $model->append(['row']);
    $expect = $store->iter_n_children(null);
    if ($expect !== 1) {
        echo "BUG: a row appended to GtkTreeView::model gave $expect rows in the store\n";
        $failed = true;
    }
}
// a store is a GObject too: its signals reach the same object as its rows
$inserted = 0;
$store->connect('row-inserted', function () use (&$inserted) {
    $inserted++;
});
$store->append(['another']);
if ($inserted !== 1) {
    echo "BUG: row-inserted on a GtkListStore ran $inserted times\n";
    $failed = true;
}

// each must throw, not crash or hand back a default
$calls = [
    'a missing property' => fn () => $button->get_property('no-such-property'),
    'a write-only property' => fn () => (new GtkCellRendererText())->get_property('background'),
    'a missing property of a store' => fn () => $store->get_property('no-such-property'),
];
foreach ($calls as $what => $call) {
    try {
        $call();
        echo "BUG: get_property() read $what\n";
        $failed = true;
    } catch (Exception $e) {
    }
}

if ($failed) {
    exit(1);
}
echo "OK\n";
