#include "php-gtk.h"

#ifndef PHPGTK_GIT_HASH
#define PHPGTK_GIT_HASH "unknown"
#endif

#ifndef PHPGTK_BUILD_DATE
#define PHPGTK_BUILD_DATE "unknown"
#endif

/**
 * Build metadata, kept in this tiny translation unit on purpose: the Makefile
 * force-rebuilds version.o on every make, so the git hash / build date injected
 * via PHPGTK_GIT_HASH / PHPGTK_BUILD_DATE stay current on incremental builds
 * without recompiling anything else.
 */
const char *phpgtk_build_info() {
  return "built " PHPGTK_BUILD_DATE ", git " PHPGTK_GIT_HASH;
}
