<?php
/**
 * A window getter that GTK documents as nullable returns null, not a GtkWindow
 * wrapping NULL; new GtkDialog($title, $parent, $flags) applies its arguments.
 *
 * GtkWindow::get_transient_for(), GtkApplication::get_window_by_id() and
 * get_active_window() and GtkWidget::get_tooltip_window() wrapped a NULL
 * window in a GtkWindow object, so "no window" could not be told apart and
 * every call on the object tripped a GTK assertion. new GtkDialog() ignored
 * its title, parent and flags (gtk_dialog_new()), so a dialog asked for as
 * modal and transient for a window was neither.
 *
 * Usage: php window_null_and_dialog_constructor.php    (needs a display on X11/Wayland, not on Windows)
 * Expected: exit status 0 and "OK" on stdout - no "BUG:" line, no crash.
 */
if (PHP_OS_FAMILY !== 'Windows'
    && getenv('DISPLAY') === false
    && getenv('WAYLAND_DISPLAY') === false
) {
    echo "SKIP: Gtk::init() needs a display\n";
    exit(0);
}

Gtk::init();

$failed = false;
$check = static function (bool $ok, string $what) use (&$failed): void {
    if (!$ok) {
        echo "BUG: {$what}\n";
        $failed = true;
    }
};

$parent = new GtkWindow();
$parent->set_title('Parent');

$window = new GtkWindow();
$check($window->get_transient_for() === null, 'get_transient_for() of a window without a parent is not null');
$window->set_transient_for($parent);
$got = $window->get_transient_for();
$check($got instanceof GtkWindow && $got->get_title() === 'Parent', 'get_transient_for() does not return the parent');

$check((new GtkLabel('x'))->get_tooltip_window() === null, 'get_tooltip_window() without a custom tooltip window is not null');

$app = new GtkApplication('io.github.phpgtk3.windownull', 0);
$check($app->get_window_by_id(4711) === null, 'get_window_by_id() of an unknown id is not null');
$check($app->get_active_window() === null, 'get_active_window() of an application without windows is not null');

$dialog = new GtkDialog('Titel', $parent, GtkDialogFlags::MODAL);
$check($dialog->get_title() === 'Titel', 'new GtkDialog() ignores its title');
// get_modal() hands back GTK's gboolean as an int
$check((bool) $dialog->get_modal(), 'new GtkDialog() ignores GtkDialogFlags::MODAL');
$dialogParent = $dialog->get_transient_for();
$check($dialogParent instanceof GtkWindow && $dialogParent->get_title() === 'Parent', 'new GtkDialog() ignores its parent');

// Without arguments, and with a null parent, as PHP subclasses and older callers do
$plain = new GtkDialog();
$check(!$plain->get_modal() && $plain->get_transient_for() === null, 'new GtkDialog() without arguments is not a plain dialog');
$noParent = new GtkDialog('Ohne', null, 0);
$check($noParent->get_title() === 'Ohne' && $noParent->get_transient_for() === null, 'new GtkDialog() with a null parent misbehaves');

class SubDialog extends GtkDialog
{
    public function __construct()
    {
        parent::__construct();
        $this->set_title('Sub');
    }
}
$check((new SubDialog())->get_title() === 'Sub', 'a GtkDialog subclass calling parent::__construct() breaks');

if ($failed) {
    exit(1);
}
echo "OK\n";
