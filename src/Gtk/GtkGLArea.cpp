// Platform-specific GtkGLArea implementations
// This file includes the appropriate implementation based on the platform
// Note: Including .cpp files is intentional here to provide platform-specific
// implementations of the same class methods without requiring separate compilation units.

#ifdef _WIN32
// Windows: Use GtkGLArea with libepoxy (from GTK3 installation)
#include "GtkGLArea_Windows.cpp"
#else
// Unix/Linux: Use GtkGLArea with libepoxy
#include "GtkGLArea_Unix.cpp"
#endif
