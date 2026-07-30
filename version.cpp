#include "php-gtk.h"

#ifndef PHPGTK_GIT_HASH
#define PHPGTK_GIT_HASH "unknown"
#endif

#ifndef PHPGTK_BUILD_DATE
#define PHPGTK_BUILD_DATE "unknown"
#endif

#ifndef PHPGTK_PHPCPP_LIB
#define PHPGTK_PHPCPP_LIB "unknown"
#endif

#define PHPGTK_STR_HELPER(x) #x
#define PHPGTK_STR(x) PHPGTK_STR_HELPER(x)

/**
 * Build metadata, kept in this tiny translation unit on purpose: the Makefile
 * force-rebuilds version.o on every make, so the git hash / build date injected
 * via PHPGTK_GIT_HASH / PHPGTK_BUILD_DATE stay current on incremental builds
 * without recompiling anything else.
 */
const char *phpgtk_build_info() {
  return "built " PHPGTK_BUILD_DATE ", git " PHPGTK_GIT_HASH;
}

/**
 * The PHP-CPP layer this binary was built against: the linked libphpcpp
 * release (from the library filename, injected by the Makefile as
 * PHPGTK_PHPCPP_LIB - "system" when linked via plain -lphpcpp) and the
 * PHPCPP_API_VERSION of the headers that were compiled in.
 */
const char *phpgtk_phpcpp_info() {
  return "libphpcpp " PHPGTK_PHPCPP_LIB ", api " PHPGTK_STR(PHPCPP_API_VERSION);
}

/**
 * The optional WITH_* features this binary was compiled with. Assembled from
 * adjacent string literals selected by the same defines the Makefile sets, so
 * the reported values can never drift from what was actually compiled in.
 */
const char *phpgtk_build_features() {
  return
#ifdef WITH_WEBKIT
      "webkit=yes"
#else
      "webkit=no"
#endif
      ", "
#ifdef WITH_GLADEUI
      "gladeui=yes"
#else
      "gladeui=no"
#endif
      ", "
#ifdef WITH_LIBWNCK
      "libwnck=yes"
#else
      "libwnck=no"
#endif
      ", "
#ifdef WITH_MAC_INTEGRATION
      "mac_integration=yes"
#else
      "mac_integration=no"
#endif
      ;
}
