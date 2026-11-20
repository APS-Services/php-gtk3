<?php
/**
 * A text argument that is not a PHP string must be converted, not crash.
 *
 * GtkLabel's and GtkFrame's constructors and insert_with_tags_by_name() read
 * their text argument as a raw C string, which only a PHP string has: an int,
 * a float or null there (new GtkLabel($row['name']) with a NULL column) was a
 * segfault. create_tag() handed GTK an unterminated property list and crashed
 * on every call. A GtkTextTag kept its tag in a field of its own, so a tag
 * made with new crashed in set_property() and one returned by GTK reached
 * insert_with_tags() as NULL.
 *
 * Usage: php non_string_arguments.php    (needs a display on X11/Wayland, not on Windows)
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

$expect('new GtkLabel(66)', (new GtkLabel(66))->get_text(), '66');
$expect('new GtkLabel(1.5)', (new GtkLabel(1.5))->get_text(), '1.5');
$expect('new GtkLabel(null)', (new GtkLabel(null))->get_text(), '');
$expect('new GtkFrame(66)', (new GtkFrame(66))->get_label(), '66');

$buffer = new GtkTextBuffer();
$tag = $buffer->create_tag('66', ['weight' => 700]);
$buffer->insert_with_tags_by_name($buffer->get_end_iter(), 'x', -1, 66);
$expect('insert_with_tags_by_name(..., 66) tag', $buffer->get_start_iter()->has_tag($tag), true);
$expect('create_tag() without a name', get_class($buffer->create_tag()), 'GtkTextTag');
// a tag made with new and one from create_tag() work with the GObject and the text API alike
$own = new GtkTextTag('own');
$own->set_property('weight', 700);
$buffer->get_tag_table()->add($own);
$buffer->insert_with_tags($buffer->get_end_iter(), 'y', -1, $tag);
$expect('insert_with_tags() with a created tag', $buffer->get_iter_at_offset(1)->has_tag($tag), true);
// each one must throw, not be skipped or crash
$calls = [
    'a second tag named 66' => fn () => $buffer->create_tag('66'),
    'an unknown property' => fn () => $buffer->create_tag('unknown', ['no-such-property' => 1]),
    'properties that are no array' => fn () => $buffer->create_tag('loose', 'foreground'),
    'a model that is no GtkTreeModel' => fn () => $buffer->create_tag('model', ['model' => 1]),
];
foreach ($calls as $what => $call) {
    try {
        $call();
        echo "BUG: create_tag() accepted $what\n";
        $failed = true;
    } catch (Exception $e) {
    }
}
// a failed create_tag() leaves no tag behind, so its name is free again
$expect('create_tag() after a failed one', get_class($buffer->create_tag('unknown')), 'GtkTextTag');

if ($failed) {
    exit(1);
}
echo "OK\n";
