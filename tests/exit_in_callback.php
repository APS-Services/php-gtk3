<?php
/**
 * exit() inside a callback must end the script (PHP 8 turns exit() into an
 * internal unwind exception that the callback boundary used to swallow).
 *
 * Usage: php exit_in_callback.php dialog|timeout
 * Expected: exit status 3 (dialog) or 4 (timeout), and on stdout
 * "shutdown function ran" and "buffered output flushed" - no "BUG:" line.
 * With the bug the script never ends, so run it under a timeout.
 */
Gtk::init();
register_shutdown_function(function () {
    echo "shutdown function ran\n";
});
ob_start();
echo "buffered output flushed\n";

$mode = $argv[1] ?? 'dialog';
if ($mode === 'dialog') {
    // A signal handler inside a dialog's run() loop, like a "Quit" button on a login dialog
    $dialog = new GtkDialog('', null, GtkDialogFlags::MODAL);
    $button = GtkButton::new_with_label('Quit');
    $button->connect('clicked', function () {
        exit(3);
    });
    $dialog->get_content_area()->add($button);
    $dialog->show_all();
    Gtk::timeout_add(100, function () use ($button) {
        $button->clicked();
        return false;
    });
    $dialog->run();
    echo "BUG: dialog->run() returned, exit was swallowed\n";
} else {
    Gtk::timeout_add(100, function () {
        exit(4);
    });
    Gtk::main();
    echo "BUG: Gtk::main() returned, exit was swallowed\n";
}
exit(1);
