<?php
/**
 * Every event struct GdkEvent::populate() fills in must be a declared property
 * of GdkEvent. An undeclared one becomes a dynamic property, which PHP 8.2+
 * reports as deprecated on every scroll, hover and pointer move - and a
 * debugger that breaks on it stops inside GTK's event dispatch.
 *
 * Usage: php event_properties.php    (no display needed)
 * Expected: exit status 0 and "OK" on stdout - no "BUG:" line.
 */
$failed = false;
$class = new ReflectionClass('GdkEvent');

// the members of the GdkEvent union that src/Gdk/GdkEvent.cpp writes
foreach (['type', 'button', 'key', 'focus_change', 'scroll', 'motion', 'crossing', 'touch', 'configure'] as $name) {
    if (!$class->hasProperty($name)) {
        echo "BUG: GdkEvent::\$$name is not declared\n";
        $failed = true;
    }
}

if ($failed) {
    exit(1);
}
echo "OK\n";
