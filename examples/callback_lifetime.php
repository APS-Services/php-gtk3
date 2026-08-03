<?php

/**
 * Verification script for the callback-userdata lifetime fixes.
 *
 * Every callback in php-gtk3 stores its PHP callable in a C struct that is
 * handed to GTK as user data. Those structs used to be created with
 * malloc() + memset() even though they hold Php::Value members - the
 * constructors never ran - and most of them were never freed at all, because
 * the GDestroyNotify/GClosureNotify slot was left NULL.
 *
 * They are now constructed with new and released through those notify slots.
 * The release path is the interesting half: it only runs when the owning object
 * goes away (a widget is destroyed, a timeout source is removed, a sort function
 * is replaced), so this script deliberately drives each of those teardowns
 * rather than just installing callbacks and exiting.
 *
 * Run it:
 *
 *   xvfb-run -a php8.4 examples/callback_lifetime.php
 *
 * and, to confirm the structs are actually freed rather than merely not
 * crashing:
 *
 *   xvfb-run -a valgrind --leak-check=full --show-leak-kinds=definite \
 *       php8.4 examples/callback_lifetime.php
 *
 * GTK and PHP both leak plenty on their own, so read the valgrind output for
 * *relative* change against the previous build rather than expecting silence.
 *
 * Exit status is 0 when every check passed, 1 otherwise.
 */

Gtk::init();

final class Report
{
    private static $checks = 0;
    private static $failures = 0;

    public static function section(string $title): void
    {
        echo "\n== " . $title . "\n";
    }

    public static function check(string $what, bool $ok, string $detail = ''): void
    {
        self::$checks++;

        if (!$ok) {
            self::$failures++;
        }

        printf("  [%s] %s%s\n", $ok ? ' ok ' : 'FAIL', $what, $detail === '' ? '' : ' (' . $detail . ')');
    }

    public static function note(string $line): void
    {
        echo "       " . $line . "\n";
    }

    public static function summary(): int
    {
        printf("\n%d checks, %d failures\n", self::$checks, self::$failures);

        return self::$failures === 0 ? 0 : 1;
    }
}

/**
 * GtkClipboard::request_text() takes the handler as a *function name*, not as a
 * callable - the binding reads it into a std::string - so these two cannot be
 * closures like the rest of the script uses.
 */
$clipboard_text = 'not called';
$clipboard_calls = 0;
$clipboard_in_loop = false;

function on_clipboard_text($clipboard, $text): void
{
    global $clipboard_text, $clipboard_calls, $clipboard_in_loop;

    $clipboard_text = $text;
    $clipboard_calls++;

    // When this process owns the selection GTK answers immediately, before the
    // main loop is ever entered - calling main_quit() then trips a Gtk-CRITICAL.
    if ($clipboard_in_loop) {
        Gtk::main_quit();
    }
}

/**
 * Request the clipboard text and return once the handler has run, driving the
 * main loop only if the answer did not already arrive synchronously.
 */
function request_clipboard_text($clipboard): void
{
    global $clipboard_text, $clipboard_in_loop;

    $clipboard_text = 'not called';
    $clipboard->request_text('on_clipboard_text');

    if ($clipboard_text !== 'not called') {
        return;
    }

    $clipboard_in_loop = true;
    Gtk::timeout_add(2000, function () {
        Gtk::main_quit();

        return false;
    });
    Gtk::main();
    $clipboard_in_loop = false;
}

// ---------------------------------------------------------------------------
Report::section('GObject::connect - closure freed when the object is destroyed');

$destroy_fired = 0;
$window = new GtkWindow();
$window->connect('destroy', function () use (&$destroy_fired) {
    $destroy_fired++;
});
$window->destroy();

Report::check('destroy handler fired exactly once', $destroy_fired === 1, 'fired=' . $destroy_fired);

// The connect/destroy cycle is where the restored GClosureNotify runs. Repeating
// it is what turns a double-free or a use-after-free into a reliable crash
// instead of an occasional one, and what makes the leak visible under valgrind.
$cycles = 200;
for ($i = 0; $i < $cycles; $i++) {
    $w = new GtkWindow();
    $w->connect('destroy', function () {
    });
    $w->connect('show', function () {
    });
    $w->destroy();
}

// Detailed signal names take a different route through connect_internal():
// g_signal_lookup() does not parse the "::detail" suffix and fails (printing a
// GLib-GObject-CRITICAL, which is expected here and predates this work), while
// g_signal_connect_closure() does parse it and succeeds. Connected once rather
// than inside the loop above, because 200 of those criticals bury the results.
$detailed = new GtkWindow();
$detailed->connect('notify::title', function () {
});
$detailed->destroy();

Report::check($cycles . ' connect/destroy cycles survived', true);
Report::note('a crash here means the closure notify is freeing something it should not own');

// ---------------------------------------------------------------------------
Report::section('Gtk::timeout_add - source data freed on removal');

// Removed explicitly: exercises the GDestroyNotify via Gtk::source_remove().
for ($i = 0; $i < 200; $i++) {
    $tag = Gtk::timeout_add(10000, function () {
        return false;
    });
    Gtk::source_remove($tag);
}

Report::check('200 timeout_add/source_remove cycles survived', true);

// Removed by returning false: the other route into the same notify.
$ticks = 0;
Gtk::timeout_add(10, function () use (&$ticks) {
    $ticks++;

    return $ticks < 3;
});

// Backstop so the script can never hang if a callback above never fires.
Gtk::timeout_add(2000, function () {
    Gtk::main_quit();

    return false;
});

Gtk::main();

Report::check('self-removing timeout ran exactly 3 times', $ticks === 3, 'ticks=' . $ticks);

// ---------------------------------------------------------------------------
Report::section('GtkClipboard::request_text - null text no longer crashes');

$clipboard = new GtkClipboard(GdkSelection::CLIPBOARD);
$clipboard->set_text('php-gtk3 lifetime check', -1);

request_clipboard_text($clipboard);

Report::check('handler received the text that was set', $clipboard_text === 'php-gtk3 lifetime check',
    'got ' . var_export($clipboard_text, true));

// The regression: GTK passes NULL when the clipboard holds nothing convertible
// to text, and that NULL used to go straight into a Php::Value.
$clipboard->clear();
request_clipboard_text($clipboard);

Report::check('empty clipboard yielded null, not a crash', $clipboard_text === null,
    'got ' . var_export($clipboard_text, true));
Report::check('handler ran for both requests', $clipboard_calls === 2, 'calls=' . $clipboard_calls);

// ---------------------------------------------------------------------------
Report::section('GtkListStore::set_sort_func - data freed when the func is replaced');

$store = new GtkListStore(GObject::TYPE_STRING);
$store->append(['cherry']);
$store->append(['apple']);
$store->append(['banana']);

$first_sort_calls = 0;
$store->set_sort_func(0, function ($model, $iter_a, $iter_b) use (&$first_sort_calls) {
    $first_sort_calls++;

    return 0;
});
$store->set_sort_column_id(0, GtkSortType::ASCENDING);

// Replacing the sort function is what fires the GDestroyNotify on the first one.
$second_sort_calls = 0;
$store->set_sort_func(0, function ($model, $iter_a, $iter_b) use (&$second_sort_calls) {
    $second_sort_calls++;

    return 0;
});
$store->set_sort_column_id(0, GtkSortType::DESCENDING);

Report::check('first sort function was called', $first_sort_calls > 0, 'calls=' . $first_sort_calls);
Report::check('replacement sort function was called', $second_sort_calls > 0, 'calls=' . $second_sort_calls);
Report::note('the replacement is what frees the first function\'s data');

// ---------------------------------------------------------------------------
Report::section('GtkTreeViewColumn::set_cell_data_func - data freed on replacement');

$renderer = new GtkCellRendererText();
$column = new GtkTreeViewColumn('Fruit', $renderer, 'text', 0);

$column->set_cell_data_func($renderer, function ($col, $cell, $model, $iter) {
});
$column->set_cell_data_func($renderer, function ($col, $cell, $model, $iter) {
});

Report::check('cell data func replaced without crashing', true);

// ---------------------------------------------------------------------------
Report::section('GtkTreeSelection::selected_foreach - callback data now on the stack');

$view = new GtkTreeView();
$view->set_model($store);
$view->append_column($column);

$selection = $view->get_selection();
$selection->select_path('0');

$visited = 0;
$selection->selected_foreach(function () use (&$visited) {
    $visited++;
});

Report::check('selected_foreach visited the selected row', $visited === 1, 'visited=' . $visited);
Report::note('this path scans varargs for its user data and is fragile independently of');
Report::note('the lifetime work - a crash here is not necessarily a regression');

// ---------------------------------------------------------------------------
Report::section('GtkAboutDialog credit lists');

$about = new GtkAboutDialog();

$about->set_authors('Alice');
$about->set_authors('Bob');

// The old code built a *static* array on the first call, so every later call
// handed GTK the first call's (long dangling) pointer. This check is the one
// that used to report Alice.
Report::check('second set_authors() took effect', $about->get_authors() === ['Bob'],
    var_export($about->get_authors(), true));

$about->set_authors(['Carol', 'Dave']);
Report::check('set_authors() accepts an array', $about->get_authors() === ['Carol', 'Dave'],
    var_export($about->get_authors(), true));

$about->set_artists(['Erin']);
Report::check('set_artists() accepts an array', $about->get_artists() === ['Erin'],
    var_export($about->get_artists(), true));

$about->set_documenters('Frank');
Report::check('set_documenters() accepts a string', $about->get_documenters() === ['Frank'],
    var_export($about->get_documenters(), true));

$about->add_credit_section('Testers', ['Grace', 'Heidi']);
Report::check('add_credit_section() accepts an array', true);

$about->destroy();

exit(Report::summary());
