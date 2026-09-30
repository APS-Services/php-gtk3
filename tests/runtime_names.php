<?php
/**
 * Strings a script hands to the extension must stay the script's: the
 * extension takes its own reference or copies, and never releases what it does
 * not own. tests/method_lookup_miss.php covers method lookups; this covers the
 * other places a name or message crosses the boundary.
 *
 * The strings are built at run time: only refcounted strings show a missing
 * reference, literals of a plain script are interned. ionCube-encoded scripts
 * have refcounted literals, which is where such bugs surface.
 *
 * Usage: php runtime_names.php probe|churn
 *   probe  property lookups that miss, a static name that misses, a callback
 *          registered by name, a signal connected by name, an exception message
 *          reported through Gtk::set_exception_handler(): every string intact
 *          afterwards, and the callbacks ran.
 *   churn  20 000 connect()/handler_disconnect() cycles must not grow the heap
 *          (connect_internal() must free what it allocates once the handler is
 *          gone).
 * Expected: exit status 0 and "OK" on stdout - no "BUG:" line, no crash.
 */
Gtk::init();

$failed = false;

/**
 * A refcounted copy of a string.
 *
 * @param string $string
 * @return string
 */
function runtimeCopy(string $string): string
{
    return str_repeat(substr($string, 0, 1), 1) . substr($string, 1);
}

/**
 * Reuses the blocks a wrongly freed string would have left behind.
 *
 * @return array<int, string>
 */
function reuseFreedBlocks(): array
{
    $blocks = [];
    for ($i = 0; $i < 200; $i++) {
        $blocks[] = str_repeat(chr(65 + $i % 26), 12) . $i % 10;
    }

    return $blocks;
}

/**
 * Reports a string that changed behind the script's back.
 *
 * @param string $label
 * @param string $actual
 * @param string $expected
 * @return void
 */
function check(string $label, string $actual, string $expected): void
{
    global $failed;

    if ($actual !== $expected) {
        echo "BUG: $label: the string became " . bin2hex($actual) . "\n";
        $failed = true;
    }
}

/**
 * Target for a callback registered by name.
 *
 * @return bool
 */
function namedCallbackTarget(): bool
{
    $GLOBALS['namedCallbackRan'] = true;

    return false;
}

$button = GtkButton::new_with_label('probe');

if (($argv[1] ?? 'probe') === 'churn') {
    $before = memory_get_usage();
    for ($i = 0; $i < 20000; $i++) {
        $id = $button->connect('clicked', function (): void {
        });
        $button->handler_disconnect($id);
    }
    $grown = memory_get_usage() - $before;
    if ($grown > 0) {
        echo "BUG: 20000 connect/disconnect cycles left $grown bytes behind\n";
        exit(1);
    }
    echo "OK\n";
    exit(0);
}

// Property lookups that miss: read, isset, write and unset of an unknown property
$name = runtimeCopy('noSuchProperty');
$value = @$button->$name;
reuseFreedBlocks();
check('property read', $name, 'noSuchProperty');

$name = runtimeCopy('noSuchProperty');
isset($button->$name);
reuseFreedBlocks();
check('property isset', $name, 'noSuchProperty');

$name = runtimeCopy('noSuchProperty');
@$button->$name = 1;
unset($button->$name);
reuseFreedBlocks();
check('property write and unset', $name, 'noSuchProperty');

// A static name that misses
$name = runtimeCopy('GtkButton::noSuchMethod');
is_callable($name);
reuseFreedBlocks();
check('static callable', $name, 'GtkButton::noSuchMethod');

// A callback registered by name, and a signal connected by name
$name = runtimeCopy('namedCallbackTarget');
Gtk::timeout_add(1, $name);
reuseFreedBlocks();
check('timeout callback name', $name, 'namedCallbackTarget');

$signal = runtimeCopy('clicked');
$clickedRan = false;
$id = $button->connect($signal, function () use (&$clickedRan): void {
    $clickedRan = true;
});
reuseFreedBlocks();
check('signal name', $signal, 'clicked');
$button->clicked();
$button->handler_disconnect($id);
if (!$clickedRan) {
    echo "BUG: the 'clicked' handler did not run\n";
    $failed = true;
}

// An exception thrown in a handler, reported through the exception handler
$message = runtimeCopy('thrown inside the clicked handler');
$reported = null;
Gtk::set_exception_handler(function (string $text, string $origin, int $code) use (&$reported): void {
    $reported = $text;
});
$id = $button->connect('clicked', function () use ($message): void {
    throw new RuntimeException($message);
});
$button->clicked();
$button->handler_disconnect($id);
Gtk::set_exception_handler(null);
reuseFreedBlocks();
check('exception message', $message, 'thrown inside the clicked handler');
if ($reported !== 'thrown inside the clicked handler') {
    echo "BUG: the exception handler got " . var_export($reported, true) . "\n";
    $failed = true;
}

// Let the timeout fire
Gtk::timeout_add(100, function (): bool {
    Gtk::main_quit();

    return false;
});
Gtk::main();
if (empty($GLOBALS['namedCallbackRan'])) {
    echo "BUG: the timeout registered by name did not run\n";
    $failed = true;
}

if ($failed) {
    exit(1);
}
echo "OK\n";
