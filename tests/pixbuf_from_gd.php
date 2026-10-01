<?php
/**
 * GdkPixbuf::new_from_gd() must turn a GD image into a pixbuf.
 *
 * It used to check instanceOf("gd"), the PHP 5 resource type, so every PHP 8
 * GdImage was rejected with "Not a GD resource"; the body behind that check
 * was debug scaffolding that var_dumped and returned 1. It now encodes the
 * image through GD and reads the bytes back with a GdkPixbufLoader.
 *
 * The rejected class name is why this mattered beyond the missing feature:
 * "gd" does not exist under PHP 8, so the lookup ran the autoloader, and
 * PHP-CPP handed it a persistent string. An autoloader that remembers the
 * names it was asked for - composer does - then kept that string as a key in
 * a request array, and PHP died with "zend_mm_heap corrupted" when it released
 * the key. Run this script without the gd extension to cover that path: the
 * call is rejected, but the process must still end cleanly.
 *
 * Usage: php pixbuf_from_gd.php           (with gd loaded: converts an image)
 *        php -n -dextension=gtk3.so pixbuf_from_gd.php
 *                                        (without gd: the autoloader path)
 * Expected: exit status 0 and "OK" on stdout - no "BUG:" line, no crash.
 */
$failed = false;

/**
 * An autoloader that keeps the name it was given, the way composer remembers
 * the classes it could not find.
 */
class Recorder
{
    /** @var array<string, bool> */
    public static $missing = [];
}
spl_autoload_register(function (string $class): void { Recorder::$missing[$class] = false; });

/**
 * Reuses the blocks a wrongly freed string would have left behind.
 *
 * @return array<int, string>
 */
function reuseFreedBlocks(): array
{
    $blocks = [];
    for ($i = 0; $i < 20000; $i++) {
        $blocks[] = str_repeat(chr(65 + $i % 26), 64);
    }

    return $blocks;
}

if (!extension_loaded('gd')) {
    // Without gd there is no GdImage, so the class lookup falls through to the
    // autoloader and the name ends up in Recorder::$missing
    try {
        GdkPixbuf::new_from_gd(new stdClass());
        echo "BUG: a plain object was accepted as a GD image\n";
        $failed = true;
    } catch (Exception $e) {
        // expected
    }

    if (!isset(Recorder::$missing['GdImage'])) {
        echo "BUG: the lookup of GdImage never reached the autoloader\n";
        $failed = true;
    }

    // Release the array while the request is still running, so the engine
    // really releases the key instead of discarding the whole request heap at
    // exit, then make the allocator work with what was freed
    Recorder::$missing = [];
    $blocks = reuseFreedBlocks();

    if ($failed) {
        exit(1);
    }
    echo "OK\n";
    exit(0);
}

// A picture with one known pixel per corner, so a conversion that loses or
// reorders the channels shows up
$width = 6;
$height = 4;
$corners = [
    [0, 0, 0xff0000],
    [$width - 1, 0, 0x00ff00],
    [0, $height - 1, 0x0000ff],
    [$width - 1, $height - 1, 0xffffff],
];

$source = imagecreatetruecolor($width, $height);
imagefill($source, 0, 0, 0x000000);
foreach ($corners as [$x, $y, $colour]) {
    imagesetpixel($source, $x, $y, $colour);
}

// The same route ka/func/func_sys.php's make_image() takes: a binary blob, a
// GD image, a pixbuf
ob_start();
imagepng($source);
$blob = ob_get_clean();

$pixbuf = GdkPixbuf::new_from_gd(imagecreatefromstring($blob));

if (!$pixbuf instanceof GdkPixbuf) {
    echo "BUG: new_from_gd() returned " . get_debug_type($pixbuf) . "\n";
    exit(1);
}
if ($pixbuf->get_width() !== $width || $pixbuf->get_height() !== $height) {
    echo "BUG: the pixbuf is {$pixbuf->get_width()}x{$pixbuf->get_height()}, expected {$width}x{$height}\n";
    $failed = true;
}

// Read the pixbuf back through GD and compare the corners
$file = tempnam(sys_get_temp_dir(), 'pixbuf') . '.png';
if ($pixbuf->save($file, 'png') !== true) {
    echo "BUG: the pixbuf could not be saved\n";
    exit(1);
}

$result = imagecreatefrompng($file);
unlink($file);

foreach ($corners as [$x, $y, $colour]) {
    $found = imagecolorat($result, $x, $y) & 0xffffff;
    if ($found !== $colour) {
        printf("BUG: pixel %d,%d is #%06x, expected #%06x\n", $x, $y, $found, $colour);
        $failed = true;
    }
}

// The rejection path must still reject, and the lookup of GdImage must not
// have reached the autoloader now that gd defines it
try {
    GdkPixbuf::new_from_gd(new stdClass());
    echo "BUG: a plain object was accepted as a GD image\n";
    $failed = true;
} catch (Exception $e) {
    // expected
}
if (isset(Recorder::$missing['GdImage'])) {
    echo "BUG: GdImage was autoloaded although gd defines it\n";
    $failed = true;
}

Recorder::$missing = [];
$blocks = reuseFreedBlocks();

if ($failed) {
    exit(1);
}
echo "OK\n";
