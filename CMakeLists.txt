# Windows build of the gtk3 extension (php_gtk3.dll) with Visual Studio.
#
# Linux and macOS keep using the Makefile; this file exists because there is
# no php-config, pkg-config or g++ toolchain to drive it on Windows. It is the
# Visual Studio project from docs/compile-windows.md written down, so that
# .github/workflows/windows.yml (and anyone with VS 2022) gets the same DLL
# without clicking it together by hand.
#
# Inputs (cache variables):
#   PHP_DEVEL_DIR   unpacked PHP "devel pack" from windows.php.net
#                   (include/, lib/php7.lib or php8.lib). Its VS/NTS/x64
#                   flavour decides which php.exe the DLL loads into, and the
#                   toolset (-T) must not be newer than that Visual Studio's.
#   PHPCPP_DIR      PHP-CPP source checkout. Its zend/ and common/ sources are
#                   compiled into the extension, like the manual build does;
#                   PHP-CPP's own CMake build still targets the PHP 7 SDK layout.
#                   windows/phpcpp-msvc.patch (calling convention of the
#                   handlers, one cast) is applied to it at configure time.
#   MSYS2_PREFIX    MSYS2 environment holding GTK (default C:/msys64/ucrt64).
#                   Headers and import libraries come from here; the runtime
#                   DLLs the extension needs at load time are its bin/.
#   WEBVIEW2_SDK    unpacked Microsoft.Web.WebView2 NuGet package. Needed when
#                   WITH_WEBKIT is on (GtkWebView on Windows is WebView2).
#   WITH_WEBKIT, WITH_GLADEUI   same switches as the Makefile (default ON).
#
# Example:
#   cmake -S . -B build -G "Visual Studio 17 2022" -A x64 ^
#         -DPHP_DEVEL_DIR=C:/php-devel -DPHPCPP_DIR=C:/PHP-CPP -DWEBVIEW2_SDK=C:/WebView2SDK
#   cmake --build build --config RelWithDebInfo

cmake_minimum_required(VERSION 3.21)
project(php-gtk3 LANGUAGES CXX)

if(NOT WIN32 OR NOT MSVC)
    message(FATAL_ERROR "This CMake build is for Visual Studio on Windows only; use the Makefile elsewhere.")
endif()

set(PHP_DEVEL_DIR "" CACHE PATH "Unpacked PHP devel pack (include/, lib/php8.lib)")
set(PHPCPP_DIR "" CACHE PATH "PHP-CPP source checkout")
set(MSYS2_PREFIX "C:/msys64/ucrt64" CACHE PATH "MSYS2 environment with GTK 3, GtkSourceView 3 and Glade")
set(WEBVIEW2_SDK "" CACHE PATH "Unpacked Microsoft.Web.WebView2 NuGet package")
option(WITH_WEBKIT "Build GtkWebView (WebView2 on Windows)" ON)
option(WITH_GLADEUI "Build the Glade widgets" ON)

foreach(var PHP_DEVEL_DIR PHPCPP_DIR)
    if(NOT IS_DIRECTORY "${${var}}")
        message(FATAL_ERROR "${var} must point to an existing directory (got '${${var}}')")
    endif()
endforeach()
# php7.lib for PHP 7.x, php8.lib for 8.x.
file(GLOB php_lib "${PHP_DEVEL_DIR}/lib/php[0-9].lib")
if(NOT php_lib)
    message(FATAL_ERROR "No lib/php<N>.lib under ${PHP_DEVEL_DIR}: PHP_DEVEL_DIR must be a PHP devel pack")
endif()
list(GET php_lib 0 php_lib)
if(WITH_WEBKIT AND NOT EXISTS "${WEBVIEW2_SDK}/build/native/include/WebView2.h")
    message(FATAL_ERROR "WITH_WEBKIT needs WEBVIEW2_SDK (build/native/include/WebView2.h); pass -DWITH_WEBKIT=OFF to build without GtkWebView")
endif()

# The static CRT is what the manual build uses (Runtime Library: /MT).
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# GTK from MSYS2: pkg-config gives the include dirs and the -l names, which
# are resolved to MinGW import libraries (lib<name>.dll.a); link.exe accepts
# those directly.
# ---------------------------------------------------------------------------
find_program(PKG_CONFIG_EXECUTABLE NAMES pkg-config pkgconf
    PATHS "${MSYS2_PREFIX}/bin" NO_DEFAULT_PATH)
if(NOT PKG_CONFIG_EXECUTABLE)
    message(FATAL_ERROR "No pkg-config in ${MSYS2_PREFIX}/bin (pacman -S mingw-w64-ucrt-x86_64-pkgconf)")
endif()
find_package(PkgConfig REQUIRED)

set(gtk_modules gtk+-3.0 gtksourceview-3.0)
if(WITH_GLADEUI)
    list(APPEND gtk_modules gladeui-2.0)
endif()
pkg_check_modules(GTK REQUIRED ${gtk_modules})

# pkg-config also lists Windows system libraries (gdi32, imm32, ...); those
# have no MSYS2 import library and are left to link.exe and the Windows SDK.
set(gtk_libraries "")
foreach(name IN LISTS GTK_LIBRARIES)
    find_library(gtk_lib_${name} NAMES "lib${name}.dll.a"
        PATHS ${GTK_LIBRARY_DIRS} "${MSYS2_PREFIX}/lib" NO_DEFAULT_PATH)
    if(gtk_lib_${name})
        list(APPEND gtk_libraries "${gtk_lib_${name}}")
    else()
        list(APPEND gtk_libraries "${name}")
    endif()
endforeach()

# ---------------------------------------------------------------------------
# PHP-CPP: <phpcpp.h> includes <phpcpp/*.h>, a layout that only `make install`
# creates. Reproduce it in the build tree instead of installing anything.
# ---------------------------------------------------------------------------
# Upstream PHP-CPP is not MSVC-clean: its handlers lack the __vectorcall the
# Zend engine uses on Windows x64, and one FreeLibrary call misses a cast.
# The patch is applied once; a checkout that already has it passes the
# reverse check.
set(phpcpp_patch "${CMAKE_SOURCE_DIR}/windows/phpcpp-msvc.patch")
find_package(Git REQUIRED)
execute_process(COMMAND "${GIT_EXECUTABLE}" apply --check "${phpcpp_patch}"
    WORKING_DIRECTORY "${PHPCPP_DIR}" RESULT_VARIABLE patch_needed OUTPUT_QUIET ERROR_QUIET)
if(patch_needed EQUAL 0)
    execute_process(COMMAND "${GIT_EXECUTABLE}" apply "${phpcpp_patch}"
        WORKING_DIRECTORY "${PHPCPP_DIR}" RESULT_VARIABLE patch_result)
    if(NOT patch_result EQUAL 0)
        message(FATAL_ERROR "Applying ${phpcpp_patch} to ${PHPCPP_DIR} failed")
    endif()
    message(STATUS "Applied ${phpcpp_patch} to ${PHPCPP_DIR}")
else()
    execute_process(COMMAND "${GIT_EXECUTABLE}" apply --reverse --check "${phpcpp_patch}"
        WORKING_DIRECTORY "${PHPCPP_DIR}" RESULT_VARIABLE patch_applied OUTPUT_QUIET ERROR_QUIET)
    if(NOT patch_applied EQUAL 0)
        message(FATAL_ERROR "${phpcpp_patch} neither applies to ${PHPCPP_DIR} nor is applied already; PHP-CPP version mismatch?")
    endif()
endif()

set(phpcpp_include "${CMAKE_BINARY_DIR}/phpcpp-include")
file(MAKE_DIRECTORY "${phpcpp_include}/phpcpp")
file(COPY "${PHPCPP_DIR}/phpcpp.h" DESTINATION "${phpcpp_include}")
file(GLOB phpcpp_headers "${PHPCPP_DIR}/include/*.h")
file(COPY ${phpcpp_headers} DESTINATION "${phpcpp_include}/phpcpp")
file(GLOB phpcpp_sources "${PHPCPP_DIR}/zend/*.cpp" "${PHPCPP_DIR}/common/*.cpp")

# ---------------------------------------------------------------------------
# Sources: the Makefile's list, minus what never builds on Windows (libwnck
# is X11, GtkosxApplication is macOS). WebKitWebView.cpp #includes the
# platform file itself, so the _Unix/_Windows files are not compiled directly.
# ---------------------------------------------------------------------------
set(source_globs
    main.cpp php-gtk.cpp version.cpp
    src/G/*.cpp src/Gdk/*.cpp src/Gtk/*.cpp src/GtkSourceView/*.cpp src/Pango/*.cpp)
if(WITH_GLADEUI)
    list(APPEND source_globs src/Glade/*.cpp)
endif()
if(WITH_WEBKIT)
    list(APPEND source_globs src/WebKit/WebKitWebView.cpp)
endif()
file(GLOB sources ${source_globs})
list(FILTER sources EXCLUDE REGEX "GtkosxApplication\\.cpp$")

# Build info baked into version.cpp, as VERSION_FLAGS does in the Makefile.
set(git_hash "unknown")
set(phpcpp_hash "unknown")
if(GIT_FOUND)
    execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" OUTPUT_VARIABLE git_hash
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
        WORKING_DIRECTORY "${PHPCPP_DIR}" OUTPUT_VARIABLE phpcpp_hash
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
endif()
string(TIMESTAMP build_date "%Y-%m-%d" UTC)

add_library(gtk3 MODULE ${sources} ${phpcpp_sources})
# php.exe resolves "extension=gtk3" to php_gtk3.dll.
set_target_properties(gtk3 PROPERTIES PREFIX "" OUTPUT_NAME "php_gtk3")

target_include_directories(gtk3 PRIVATE
    "${PHP_DEVEL_DIR}/include"
    "${PHP_DEVEL_DIR}/include/main"
    "${PHP_DEVEL_DIR}/include/Zend"
    "${PHP_DEVEL_DIR}/include/TSRM"
    "${PHP_DEVEL_DIR}/include/ext"
    "${phpcpp_include}"
    ${GTK_INCLUDE_DIRS})

# The defines of the manual build (docs/compile-windows.md). The *_COMPILATION
# ones make the GLib/GTK headers usable from MSVC; _NOEXCEPT=noexcept replaces
# PHP-CPP's MSVC spelling, which does not compile; NOMINMAX keeps windows.h
# (pulled in by the Zend headers) from breaking std::min.
#
# PHPCPP_EXPORT is empty on purpose: PHP-CPP lives inside this DLL, so nothing
# of it is imported or exported, and MSVC rejects dllexport on its thread_local
# streams and on functions that value.h befriended without it. The one symbol
# php.exe looks up, get_module, is exported by the linker below.
target_compile_definitions(gtk3 PRIVATE
    _WINDOWS ZEND_DEBUG=0 PHP_WIN32 ZEND_WIN32 ZEND_WIN32_KEEP_INLINE
    _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE _NOEXCEPT=noexcept NOMINMAX
    GLIB_COMPILATION GIO_COMPILATION GDK_PIXBUF_COMPILATION
    GDK_COMPILATION GTK_COMPILATION PHPCPP_EXPORT=
    PHPGTK_GIT_HASH="${git_hash}"
    PHPGTK_BUILD_DATE="${build_date}"
    PHPGTK_PHPCPP_LIB="PHP-CPP ${phpcpp_hash}")
if(WITH_GLADEUI)
    target_compile_definitions(gtk3 PRIVATE WITH_GLADEUI)
endif()

# /bigobj: main.cpp registers every class in one translation unit.
# /W1 instead of the manual build's /W0, so real problems still show up
# without the GTK headers' warning flood.
target_compile_options(gtk3 PRIVATE /EHsc /MP /bigobj /utf-8 /W1 /sdl-)

target_link_options(gtk3 PRIVATE /EXPORT:get_module)
target_link_libraries(gtk3 PRIVATE
    "${php_lib}"
    ${gtk_libraries})

if(WITH_WEBKIT)
    target_compile_definitions(gtk3 PRIVATE WITH_WEBKIT)
    target_include_directories(gtk3 PRIVATE "${WEBVIEW2_SDK}/build/native/include")
    target_link_libraries(gtk3 PRIVATE
        "${WEBVIEW2_SDK}/build/native/x64/WebView2LoaderStatic.lib"
        # WebView2LoaderStatic's own imports.
        advapi32 version ole32 shlwapi user32)
endif()

install(TARGETS gtk3 LIBRARY DESTINATION .)
install(FILES "$<TARGET_PDB_FILE:gtk3>" "${CMAKE_SOURCE_DIR}/gtk3.ini" DESTINATION . OPTIONAL)
