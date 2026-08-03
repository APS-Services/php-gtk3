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
 * GtkClipboard::request_text() accepts any callable - a closure here, which is
 * the case that used to fail: the handler was read into a std::string, so only
 * a plain function name worked and a closure produced a conversion error.
 */
$clipboard_text = 'not called';
$clipboard_calls = 0;
$clipboard_in_loop = false;

/**
 * Request the clipboard text and return once the handler has run, driving the
 * main loop only if the answer did not already arrive synchronously.
 */
function request_clipboard_text($clipboard): void
{
    global $clipboard_text, $clipboard_calls, $clipboard_in_loop;

    $clipboard_text = 'not called';
    $clipboard->request_text(function ($clip, $text) {
        global $clipboard_text, $clipboard_calls, $clipboard_in_loop;

        $clipboard_text = $text;
        $clipboard_calls++;

        // When this process owns the selection GTK answers immediately, before
        // the main loop is entered - main_quit() then trips a Gtk-CRITICAL.
        if ($clipboard_in_loop) {
            Gtk::main_quit();
        }
    });

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
Report::section('GtkClipboard::request_text - closures and null text');

$clipboard = new GtkClipboard(GdkSelection::CLIPBOARD);
$clipboard->set_text('php-gtk3 lifetime check', -1);

// The handler is a closure - see request_clipboard_text(). That is the case the
// std::string conversion used to reject outright.
request_clipboard_text($clipboard);

Report::check('closure handler received the text that was set', $clipboard_text === 'php-gtk3 lifetime check',
    'got ' . var_export($clipboard_text, true));

// The regression: GTK passes NULL when the clipboard holds nothing convertible
// to text, and that NULL used to go straight into a Php::Value.
$clipboard->clear();
request_clipboard_text($clipboard);

Report::check('empty clipboard yielded null, not a crash', $clipboard_text === null,
    'got ' . var_export($clipboard_text, true));
Report::check('handler ran for both requests', $clipboard_calls === 2, 'calls=' . $clipboard_calls);

// A method callable must work too, not just a closure.
class ClipboardSink
{
    public $text = 'not called';

    public function receive($clip, $text): void
    {
        $this->text = $text;
    }
}

$sink = new ClipboardSink();
$clipboard->set_text('array callable', -1);
$clipboard->request_text([$sink, 'receive']);
Report::check('[$object, method] callable accepted', $sink->text === 'array callable',
    var_export($sink->text, true));

// A non-callable must be rejected in PHP space, where it can still be thrown.
$rejected = false;
try {
    $clipboard->request_text('this_function_does_not_exist');
} catch (Throwable $e) {
    $rejected = true;
}
Report::check('request_text rejects a non-callable', $rejected);

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
Report::section('GtkTreeSelection::selected_foreach - typed callback');

$view = new GtkTreeView();
$view->set_model($store);
$view->append_column($column);

$selection = $view->get_selection();
$selection->select_path('0');

// The handler now receives the arguments GTK actually passes. Previously this
// went through a generic marshaller that scanned its varargs hunting for the
// user data, reinterpreting the path and iter as PHP values on the way, and
// delivered neither of them to PHP.
$visited = 0;
$seen_model = null;
$seen_path = null;
$seen_iter = null;
$seen_extra = null;

$selection->selected_foreach(function ($model, $path, $iter, $extra) use (
    &$visited, &$seen_model, &$seen_path, &$seen_iter, &$seen_extra
) {
    $visited++;
    $seen_model = $model;
    $seen_path = $path;
    $seen_iter = $iter;
    $seen_extra = $extra;
}, 'user-data');

Report::check('selected_foreach visited the selected row', $visited === 1, 'visited=' . $visited);
Report::check('handler received the GtkTreeModel', $seen_model instanceof GtkTreeModel,
    is_object($seen_model) ? get_class($seen_model) : gettype($seen_model));
Report::check('handler received the row path', $seen_path === '0', var_export($seen_path, true));
Report::check('handler received a GtkTreeIter', $seen_iter instanceof GtkTreeIter,
    is_object($seen_iter) ? get_class($seen_iter) : gettype($seen_iter));
Report::check('handler received the user parameter', $seen_extra === 'user-data',
    var_export($seen_extra, true));

// A non-callable must be rejected in PHP space, where it can still be thrown.
$rejected = false;
try {
    $selection->selected_foreach('this_function_does_not_exist');
} catch (Throwable $e) {
    $rejected = true;
}
Report::check('selected_foreach rejects a non-callable', $rejected);

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
