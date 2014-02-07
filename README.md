# Nintendont

Nintendont is an open source GameCube loader for Wii and Wii U. 

## Building from source

Prerequisites:

 * Bourne-compatible shell
 * GNU Make 
 * devkitARM
 * devkitPPC
 * libogc

### Bourne-compatible shell

A Bourne-compatible shell is included in OS X and most Linux distributions. 
Windows users are recommended to install [MSYS](http://www.mingw.org/wiki/Getting_Started).

### GNU Make

OS X users will have to install Xcode. Linux users 
For Windows users, make is available via the MinGW package manager (mingw-get) if not already installed.

### devkitARM
Please follow the [devkitARM setup instructions](http://devkitpro.org/wiki/Getting_Started/devkitARM) (only the toolchain itself is needed). Make sure that the `DEVKITARM` environment variable is defined as instructed.

### devkitPPC and libogc
Please follow the [devkitPPC setup instructions](http://devkitpro.org/wiki/Getting_Started/devkitPPC) (including libogc). Make sure that the `DEVKITPPC` environment variable is defined as instructed.

### Building the kernel

With everything prepared, issue the following command from the root of the source tree:

`make -C kernel`

### Building the loader

With everything prepared, issue the following command from the root of the source tree:

`make -C loader LIBOGC_INC=<path-to-libogc>/include LIBOGC_LIB=<path-to-libogc>/lib/wii`
