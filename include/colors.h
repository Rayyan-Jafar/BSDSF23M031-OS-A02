#ifndef COLORS_H
#define COLORS_H

// ===== ANSI Color Codes for ls =====
#define COLOR_RESET      "\033[0m"
#define COLOR_BLUE       "\033[0;34m"   // directories
#define COLOR_GREEN      "\033[0;32m"   // executables
#define COLOR_RED        "\033[0;31m"   // archives (.tar, .gz, .zip)
#define COLOR_PINK       "\033[0;35m"   // symbolic links
#define COLOR_REVERSE    "\033[7m"      // special files (devices, sockets, etc.)

#endif
