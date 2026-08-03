#!/bin/bash
set -e

# Re-run with sudo if not root
if [ "$EUID" -ne 0 ]; then
    exec sudo PHPCPP_BASE="$PHPCPP_BASE" "$0" "$@"
fi

# Base directory containing per-version libphpcpp folders (e.g. php7.4/, php8.4/)
# Override by setting PHPCPP_BASE before running, e.g.:
#   PHPCPP_BASE=/custom/path ./build-all.sh
PHPCPP_BASE="${PHPCPP_BASE:-/mnt/share/dev/code/PHP-CPP-FORK/dist}"

# FORMAT: "version:php_config_path:ini_dir:so_dest:with_webkit:enabled"
# so_dest: system extension dir where php8.x binary loads extensions from
# enabled: 1=build by default, 0=skip by default
#
# The php-config here must be the same one PHP-CPP was built against
# (PHP-CPP/build-dist.sh uses php-config<version> from PATH), otherwise the
# extension is compiled against one PHP's headers and linked against a
# libphpcpp compiled against another's. That mismatch is not caught at build
# time: it produces a working-looking .so that corrupts the heap and aborts
# the process during module shutdown.
#
# These are the distro packages - /usr/bin/php-config7.4 is PHP 7.4.33 and
# /usr/bin/php-config8.4 is PHP 8.4.24 - matching the so_dest directories the
# system php binaries actually load from. The /opt/php trees are not used:
# they hold different patch levels (8.4.14-dev, and a 7.4.31-dev binary living
# in a directory named php-7.4.33).
BUILDS=(
    "7.4:/usr/bin/php-config7.4:/etc/php/7.4/mods-available:/usr/lib/php/20190902:0:1"
   # "8.2:/usr/bin/php-config8.2:/etc/php/8.2/mods-available:/usr/lib/php/20220829:1:0"
   # "8.3:/usr/bin/php-config8.3:/etc/php/8.3/mods-available:/usr/lib/php/20230831:1:0"
    "8.4:/usr/bin/php-config8.4:/etc/php/8.4/mods-available:/usr/lib/php/20240924:0:1"
)

# Kill any running php processes, but don't fail if none exist
for BUILD in "${BUILDS[@]}"; do
    VERSION=$(echo "$BUILD" | cut -d: -f1)
    ENABLED=$(echo "$BUILD" | cut -d: -f6)
    [ "$ENABLED" -eq 1 ] && pkill -f "/usr/bin/php${VERSION}" 2>/dev/null || true
done

#php -vgit pull

for BUILD in "${BUILDS[@]}"; do
    VERSION=$(echo     "$BUILD" | cut -d: -f1)
    PHP_CONFIG=$(echo  "$BUILD" | cut -d: -f2)
    INI_DIR=$(echo     "$BUILD" | cut -d: -f3)
    SO_DEST=$(echo     "$BUILD" | cut -d: -f4)
    WITH_WEBKIT=$(echo "$BUILD" | cut -d: -f5)
    ENABLED=$(echo     "$BUILD" | cut -d: -f6)

    if [ "$ENABLED" -ne 1 ]; then
        echo "=== Skipping PHP ${VERSION} (disabled) ==="
        continue
    fi

    PHPCPP_DIR="${PHPCPP_BASE}/php${VERSION}"
    # Highest version wins. When several libphpcpp.a.* sit side by side, taking
    # whichever the filesystem happens to list first silently links an old one -
    # that is how a stale 2.4.13 got linked while 2.4.16 sat next to it.
    PHPCPP_STATIC=$(find "${PHPCPP_DIR}" -maxdepth 1 -name "libphpcpp.a.*" | sort -V | tail -1)

    if [ -z "$PHPCPP_STATIC" ]; then
        echo "ERROR: No static libphpcpp found in ${PHPCPP_DIR}" >&2
        exit 1
    fi

    echo "=== Building for PHP ${VERSION} (libphpcpp: ${PHPCPP_STATIC}) ==="
    make clean
    make PHP_CONFIG="${PHP_CONFIG}" INI_DIR="${INI_DIR}/" WITH_WEBKIT="${WITH_WEBKIT}" PHPCPP_STATIC="${PHPCPP_STATIC}" -j12
    cp gtk3.so "${SO_DEST}"
done

echo "=== All versions built successfully ==="
