<?php
/**
 * What connect(), Gtk::timeout_add() and the other callback setters keep for a
 * callback (the callable, its parameters, the PHP object of the widget) lives
 * exactly as long as the callback can still be called: it is released when the
 * handler is disconnected, the widget destroyed or the timeout ended - and not
 * a moment earlier, even when a handler disconnects itself or destroys its own
 * widget while it runs.
 *
 * A callable is "released" when its closure is destroyed, which a marker
 * object captured by the closure reports from its destructor.
 *
 * Usage: php callback_lifetime.php
 * Expected: exit status 0 and "OK" on stdout - no "BUG:" line, no crash.
 */
Gtk::init();

$failed = false;
$destroyed = 0;

/**
 * A closure whose destruction is counted in $destroyed.
 *
 * @param callable|null $body What the closure does when called
 * @return Closure
 */
function counted(?callable $body = null): Closure
{
    $marker = new class {
        public function __destruct()
        {
            $GLOBALS['destroyed']++;
        }
    };

    return function (...$arguments) use ($marker, $body) {
        return $body === null ? false : $body(...$arguments);
    };
}

/**
 * Reports a count that is not the expected one.
 *
 * @param string $label
 * @param int $actual
 * @param int $expected
 * @return void
 */
function expect(string $label, int $actual, int $expected): void
{
    global $failed;

    if ($actual !== $expected) {
        echo "BUG: $label: $actual, expected $expected\n";
        $failed = true;
    }
}

/**
 * Runs the main loop until it is idle for the given time.
 *
 * @param int $milliseconds
 * @return void
 */
function settle(int $milliseconds): void
{
    Gtk::timeout_add($milliseconds, function (): bool {
        Gtk::main_quit();

        return false;
    });
    Gtk::main();
}

// A disconnected handler is released, a connected one is kept and still runs
$button = GtkButton::new_with_label('lifetime');
$destroyed = 0;
for ($i = 0; $i < 100; $i++) {
    $button->handler_disconnect($button->connect('clicked', counted()));
}
expect('closures released after 100 connect/disconnect', $destroyed, 100);

$ran = 0;
$destroyed = 0;
$id = $button->connect('clicked', counted(function () use (&$ran): void {
    $ran++;
}));
$button->clicked();
expect('a connected handler ran', $ran, 1);
expect('a connected handler is kept', $destroyed, 0);
$button->handler_disconnect($id);
expect('it is released once disconnected', $destroyed, 1);

// A handler that disconnects itself finishes first
$destroyed = 0;
$after = 0;
$id = $button->connect('clicked', counted(function (GtkButton $button) use (&$id, &$after): void {
    $button->handler_disconnect($id);
    $after++;
}));
$button->clicked();
$button->clicked();
expect('a handler that disconnects itself ran once, to its end', $after, 1);
expect('and is released afterwards', $destroyed, 1);

// 20 000 cycles leave nothing behind
$before = memory_get_usage();
for ($i = 0; $i < 20000; $i++) {
    $button->handler_disconnect($button->connect('clicked', function (): void {
    }));
}
expect('bytes left by 20000 connect/disconnect cycles', memory_get_usage() - $before, 0);

// Destroying a widget releases its handlers, also from inside one of them
$window = new GtkWindow();
$inner = GtkButton::new_with_label('inner');
$window->add($inner);
$destroyed = 0;
$inner->connect('clicked', counted());
$window->connect('destroy', counted());
$window->destroy();
expect('handlers released with their destroyed window', $destroyed, 2);

$window = new GtkWindow();
$inner = GtkButton::new_with_label('inner');
$window->add($inner);
$destroyed = 0;
$after = 0;
$inner->connect('clicked', counted(function () use ($window, &$after): void {
    $window->destroy();
    $after++;
}));
$inner->clicked();
expect('a handler that destroys its window ran to its end', $after, 1);
expect('and is released afterwards', $destroyed, 1);
unset($window, $inner);

// Timeouts: released when they end, when they are removed, and not before
$destroyed = 0;
$fired = 0;
for ($i = 0; $i < 100; $i++) {
    Gtk::timeout_add(1, counted(function () use (&$fired): bool {
        $fired++;

        return false;
    }));
}
$pending = Gtk::timeout_add(60000, counted());
settle(200);
expect('one-shot timeouts fired', $fired, 100);
expect('one-shot timeouts released, the pending one kept', $destroyed, 100);
Gtk::source_remove($pending);
expect('the removed timeout released', $destroyed, 101);

$before = memory_get_usage();
for ($i = 0; $i < 20000; $i++) {
    Gtk::source_remove(Gtk::timeout_add(60000, function (): bool {
        return false;
    }));
}
expect('bytes left by 20000 timeout_add/source_remove cycles', memory_get_usage() - $before, 0);

// A sort function is released when it is replaced
$store = new GtkListStore(GObject::TYPE_STRING);
$store->append(['b']);
$store->append(['a']);
$destroyed = 0;
$store->set_sort_func(0, counted(fn (): int => 0));
$store->set_sort_column_id(0, GtkSortType::ASCENDING);
expect('a sort function in use is kept', $destroyed, 0);
$store->set_sort_func(0, counted(fn (): int => 0));
expect('a replaced sort function is released', $destroyed, 1);

if ($failed) {
    exit(1);
}
echo "OK\n";
