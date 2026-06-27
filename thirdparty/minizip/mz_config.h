#ifndef MZ_CONFIG_H
#define MZ_CONFIG_H

// Define to 1 if you have the <dirent.h> header file.
#if defined(__APPLE__)
#  define HAVE_DIRENT_H 1
#else
#  define HAVE_DIRENT_H 0
#endif

// Define to 1 if you have the <sys/dirent.h> header file.
#define HAVE_SYS_DIRENT_H 0

// Define to 1 if you have the <inttypes.h> header file.
#define HAVE_INTTYPES_H 0

// Define to 1 if you have the <stdint.h> header file.
#define HAVE_STDINT_H 1

// Define to 1 if DIR* is defined.
#define HAVE_PDIR 0

// Define to 1 if fseeko() is defined.
#define HAVE_FSEEKO 0

// Define to 1 if symlink() is defined.
#define HAVE_SYMLINK 0

// Define to 1 if readlink() is defined.
#define HAVE_READLINK 0

#endif
