#!/bin/bash
set -e

# Re-run with sudo if not root
if [ "$EUID" -ne 0 ]; then
    exec sudo PHPCPP_BASE="$PHPCPP_BASE" "$0" "$@"
fi

# Base directory containing per-version libphpcpp folders (e.g. php8.4/)
# Override by setting PHPCPP_BASE before running, e.g.:
#   PHPCPP_BASE=/custom/path ./buildall.sh
# Layout: <base>/include/phpcpp.h + <base>/include/phpcpp/*.h (PHP-CPP's headers as `make install` lays them out,
# the same for every PHP) and <base>/php<ver>/libphpcpp.a.<x.y.z>, each built against that PHP:
#   make clean && make release PHP_CONFIG=php-config<ver> && mkdir -p dist/php<ver> && cp libphpcpp.a.* dist/php<ver>/
# in a checkout of APS-Services/PHP-CPP. `make clean` only removes the current version's archive: an older
# libphpcpp.a.* left in the checkout must not be copied along (the newest one is linked, below).
PHPCPP_BASE="${PHPCPP_BASE:-/mnt/share/dev/code/PHP-CPP/dist}"

# FORMAT: "version:php_config_path:ini_dir:so_dest:with_webkit:enabled"
# so_dest: system extension dir where php8.x binary loads extensions from
# enabled: 1=build by default, 0=skip by default
BUILDS=(
   # "8.2:/opt/php/php-8.2.29/bin/php-config:/etc/php/8.2/mods-available:/usr/lib/php/20220829:1:0"
   # "8.3:/opt/php/php8.3/bin/php-config:/etc/php/8.3/mods-available:/usr/lib/php/20230831:1:0"
    # The distro php-config of each version, the one PHP-CPP was built against (a mismatch loads, then corrupts
    # the heap at shutdown). WebKit on, as the Windows build and CI build it.
    "8.4:/usr/bin/php-config8.4:/etc/php/8.4/mods-available:/usr/lib/php/20240924:1:1"
    "8.5:/usr/bin/php-config8.5:/etc/php/8.5/mods-available:/usr/lib/php/20250925:1:1"
    "8.6:/usr/bin/php-config8.6:/etc/php/8.6/mods-available:/usr/lib/php/20261112:1:1"
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
    PHPCPP_STATIC=$(find "${PHPCPP_DIR}" -maxdepth 1 -name "libphpcpp.a.*" | sort -V | tail -1)

    if [ -z "$PHPCPP_STATIC" ]; then
        echo "ERROR: No static libphpcpp found in ${PHPCPP_DIR}" >&2
        exit 1
    fi

    echo "=== Building for PHP ${VERSION} (libphpcpp: ${PHPCPP_STATIC}) ==="
    make clean
    # PHP-CPP's own headers, not a system /usr/include/phpcpp.h of another version
    CPLUS_INCLUDE_PATH="${PHPCPP_BASE}/include" \
        make PHP_CONFIG="${PHP_CONFIG}" INI_DIR="${INI_DIR}/" WITH_WEBKIT="${WITH_WEBKIT}" PHPCPP_STATIC="${PHPCPP_STATIC}" -j12
    cp gtk3.so "${SO_DEST}"
done

echo "=== All versions built successfully ==="
