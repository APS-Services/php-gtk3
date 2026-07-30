<?php

/**
 * Example: Using Gtk::set_exception_handler
 *
 * When a PHP signal handler throws, the exception cannot be allowed to
 * propagate: it would have to unwind across GLib's C signal-emission frames,
 * which is undefined behaviour and leaves the main loop in an inconsistent
 * state. php-gtk3 therefore catches it at the C++/PHP boundary, which means a
 * try/catch around Gtk::main() will never see it.
 *
 * Gtk::set_exception_handler() is how an application observes those failures
 * instead - typically to route them into its own logging.
 *
 * Function signature:
 * Gtk::set_exception_handler(?callable $handler)
 *
 * The handler receives:
 *   $message - the exception message
 *   $origin  - the signal whose handler threw; for non-signal callbacks the
 *              installing method, e.g. 'Gtk::timeout_add' ('' if unknown)
 *   $code    - the exception code
 *
 * Note: only these three values are available. PHP-CPP does not expose the
 * original Throwable object, so the exception class, file, line and stack trace
 * cannot be recovered. Catch inside your own handler if you need them.
 *
 * Pass null to remove a previously installed handler. With no handler
 * installed, failures are reported with g_critical() on stderr.
 */
Gtk::init();

Gtk::set_exception_handler(function (string $message, string $origin, int $code): void {
    // Keep this handler simple and total - it is the last line of defence.
    // If it throws, php-gtk3 falls back to g_critical() on stderr.
    error_log(sprintf("[php-gtk3] '%s' handler failed (code %d): %s", $origin, $code, $message));
});

// Create main window
$window = new GtkWindow();
$window->set_title('Exception Handler Example');
$window->set_default_size(400, 120);
$window->connect('destroy', function () {
    Gtk::main_quit();
});

$box = new GtkBox(GtkOrientation::VERTICAL, 6);
$box->set_border_width(12);
$window->add($box);

$label = new GtkLabel('Click a button - the exception is reported, not lost.');
$box->pack_start($label, false, false, 0);

// This handler throws. Without a handler installed, you would see a
// g_critical() line on stderr; with one, the callback above runs instead.
$throwing = new GtkButton('Throw an Exception');
$throwing->connect('clicked', function () {
    throw new RuntimeException('something went wrong in the click handler', 42);
});
$box->pack_start($throwing, false, false, 0);

// Errors are reported too, not just exceptions: php-gtk3 catches
// Php::Throwable, so a PHP Error (TypeError, undefined constant, ...) is
// handled the same way.
$erroring = new GtkButton('Trigger a PHP Error');
$erroring->connect('clicked', function () {
    UNDEFINED_CONSTANT_EXAMPLE;
});
$box->pack_start($erroring, false, false, 0);

// The application keeps running in both cases - a failed handler returns FALSE
// so it does not also block GTK's default behaviour for that signal.
$window->show_all();
Gtk::main();
