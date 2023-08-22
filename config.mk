# gasnet conduit. Must be the par version.
GASNET_CONDUIT = /usr/local/gasnet/include/mpi-conduit/mpi-par.mak

# paths
DESTDIR   =
PREFIX    = /usr/local
INCPREFIX = $(PREFIX)/include
LIBPREFIX = $(PREFIX)/lib
MANPREFIX = $(PREFIX)/share/man

# names
ANAME     = libshray.a
SONAME    = libshray.so.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)
BINSUFFIX =

# flags
CPPFLAGS =
CFLAGS   = -O3 -march=native -mtune=native -Wall -Wextra -Wpedantic
LDFLAGS  = -s

BUILD_CPPFLAGS = $(CPPFLAGS)
BUILD_CFLAGS   = $(CFLAGS)
BUILD_LDFLAGS  = $(LDFLAGS)

SHFLAGS = -fPIC -ffreestanding
SOFLAGS = -shared -nostdlib -Wl,--soname=libshray.so.$(VERSION_MAJOR).$(VERSION_MINOR)
SOSYMLINK = true

# Tools
CC	     = gcc
AR       = ar
RANLIB   = ranlib
LDCONFIG = ldconfig
SH       = sh
