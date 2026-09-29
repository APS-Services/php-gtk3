<?php
/**
 * Looking up a method that an extension object does not have must leave the
 * caller's method name alone (PHP-CPP handed it to PHP as owned by the __call
 * trampoline without a reference, so method_exists()/is_callable() freed it;
 * for a static name it handed over a null pointer).
 *
 * The names are built at run time: only refcounted strings are affected,
 * literals of a plain script are interned. ionCube-encoded scripts have
 * refcounted literals, which is where this showed up.
 *
 * Usage: php method_lookup_miss.php probe|call
 * Expected: probe: exit status 0 and "OK" on stdout - no "BUG:" line, no crash.
 *           call:  exit status 255 and "Undefined method attachEvents".
 */
$object = GdkRGBA::parse('#ff0000');
$expected = 'attachEvents';

/**
 * A refcounted copy of the method name.
 *
 * @return string
 */
function runtimeName(): string
{
    return str_repeat('attachEv', 1) . 'ents';
}

/**
 * Reuses the blocks a wrongly freed name would have left behind.
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

if (($argv[1] ?? 'probe') === 'call') {
    $name = runtimeName();
    $object->$name();
    echo "BUG: the call to a missing method returned\n";
    exit(1);
}

$failed = false;
$probes = [
    'method_exists' => function (string $name) use ($object): void {
        method_exists($object, $name);
    },
    'is_callable, object' => function (string $name) use ($object): void {
        is_callable([$object, $name]);
    },
    'is_callable, static' => function (string $name): void {
        is_callable('GdkRGBA::' . $name);
    },
];
foreach ($probes as $label => $probe) {
    $name = runtimeName();
    $probe($name);
    $blocks = reuseFreedBlocks();
    if ($name !== $expected) {
        echo "BUG: $label: the method name became " . bin2hex($name) . "\n";
        $failed = true;
    }
}

// Every trampoline owns its name: none may be left behind either
$name = runtimeName();
$before = memory_get_usage();
for ($i = 0; $i < 100000; $i++) {
    method_exists($object, $name);
    is_callable([$object, $name]);
}
$leaked = memory_get_usage() - $before;
if ($leaked !== 0) {
    echo "BUG: 100000 lookups left $leaked bytes behind\n";
    $failed = true;
}

if ($failed) {
    exit(1);
}
echo "OK\n";
