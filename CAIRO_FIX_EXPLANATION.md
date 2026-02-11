# Fix for "unresolved external symbol __imp_cairo_paint" on Windows

## Problem

When compiling PHP-GTK3 on Windows, users encountered the following linker error:
```
unresolved external symbol __imp_cairo_paint
```

This error occurred because the Cairo library was not being explicitly linked during the build process on Windows, even though:
- The code uses Cairo functions directly (e.g., `cairo_paint()`, `cairo_set_source_pixbuf()` in `src/Gdk/Gdk.cpp`)
- The Cairo DLL runtime files were already documented as runtime dependencies
- Linux builds worked fine (because `pkg-config --libs gtk+-3.0` automatically includes Cairo)

## Root Cause

On **Linux/Unix**, the Makefile uses `pkg-config` to automatically determine all required libraries:
```makefile
GTKLIBS = `pkg-config --libs gtk+-3.0 gladeui-2.0 gtksourceview-3.0 ...`
```

This command automatically includes Cairo libraries because GTK3 depends on Cairo, so the linker gets all necessary libraries transitively.

On **Windows** with Visual Studio, the build process requires **manual specification** of all import libraries in the Visual Studio project settings. The Windows compilation documentation was missing the Cairo import libraries from the linker input dependencies list.

## Solution

Added the missing Cairo import libraries to the Windows build documentation:

### In `docs/compile-windows.md` (lines 292-293):
```
libcairo-2.dll.a
libcairo-gobject-2.dll.a
```

### In `docs/compile-windows-old.md` (lines 177-178):
```
libcairo-2.dll.a
libcairo-gobject-2.dll.a
```

These import libraries (`.dll.a` files) are required at **link time** to resolve the Cairo function symbols. They correspond to the runtime DLLs that were already documented:
- `libcairo-2.dll` (runtime)
- `libcairo-gobject-2.dll` (runtime)

## How to Apply the Fix

If you're experiencing the `unresolved external symbol __imp_cairo_paint` error, follow these steps:

1. Ensure you have MSYS2 installed with GTK3 packages (as per the documentation)

2. Open your Visual Studio project for PHP-GTK3

3. Go to **Project Properties** → **Linker** → **Input** → **Additional Dependencies**

4. Add these two libraries to the list:
   ```
   libcairo-2.dll.a
   libcairo-gobject-2.dll.a
   ```

5. Ensure your **Additional Library Directories** includes:
   ```
   C:\msys2\mingw64\lib
   ```

6. Rebuild the solution

The linker should now be able to resolve all Cairo symbols, including `cairo_paint`, `cairo_set_source_pixbuf`, and any other Cairo functions used by the extension.

## Technical Details

### Cairo Usage in PHP-GTK3

Cairo functions are used in the following locations:

- **`src/Gdk/Gdk.cpp`**: Implements `Gdk::cairo_paint()` and `Gdk::cairo_set_source_pixbuf()` wrapper methods
- **`src/Gdk/Gdk.h`**: Declares the Cairo wrapper methods
- **`examples/cairo_pixbuf_example.php`**: PHP example demonstrating Cairo usage

### Why This Works on Linux but Not Windows

**Linux/Unix (using Makefile):**
- Uses `pkg-config --libs gtk+-3.0` which automatically includes all transitive dependencies
- Cairo is a dependency of GTK3, so it's automatically included
- No manual library specification needed

**Windows (using Visual Studio):**
- All libraries must be explicitly listed in the project configuration
- `pkg-config` is not used during the Visual Studio build process
- Import libraries (`.dll.a`) must be specified for the linker to resolve symbols

### Import Libraries vs Runtime DLLs

On Windows with MinGW/MSYS2:
- **Import libraries** (`.dll.a` or `.lib` files) are used at **link time** by the linker to resolve symbols
- **Runtime DLLs** (`.dll` files) are used at **runtime** when the program executes
- Both are required for a working build, but they serve different purposes

The documentation was already complete for runtime DLLs, but missing the import libraries for Cairo.

## Verification

After applying this fix, the compilation should succeed without the Cairo-related linker errors. The built extension will properly export the Cairo wrapper functions to PHP, allowing PHP scripts to use Cairo drawing operations through the GDK interface.

To verify the fix works:
1. Build the extension with the updated linker dependencies
2. Run the example: `examples/cairo_pixbuf_example.php`
3. Confirm that `Gdk::cairo_paint()` and `Gdk::cairo_set_source_pixbuf()` work without errors
