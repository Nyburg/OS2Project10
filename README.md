# Very, Very Simple File System

## Description

This project implements the next part of a Very, Very Simple File System, or VVSFS. The file system is simulated using a regular disk image file. The project currently supports opening and closing the disk image, reading and writing fixed-size logical blocks, tracking free blocks and inodes, creating the root directory, reading directory entries, and listing the root directory.

Each logical block is 4096 bytes.

## Files

- image.c and image.h: contain functions for opening and closing the disk image.
- block.c and block.h: contain functions for reading and writing logical blocks.
- free.c and free.h: contain functions for setting and finding free bits in bitmap blocks.
- inode.c and inode.h: contain functions for reading, writing, allocating, getting, and putting inodes.
- mkfs.c and mkfs.h: contain functions for initializing the file system and creating the root directory.
- dir.c and dir.h: contain directory entry constants, directory structs, and directory open/read/close functions.
- ls.c and ls.h: contain a simple function for listing the entries in the root directory.
- pack.c and pack.h: contain helper functions for reading and writing packed integer values.
- testfs.c: contains tests for image, block, free map, inode, mkfs, and directory functionality.
- ctest.h: contains simple test macros used by testfs.c.
- Makefile: builds the static library and test executable.

## Building

To build the project, run:
```sh
make
```
This buildes the object files, creates the static library `libvvsfs.a` and builds the test executable `testfs`

## Testing

To build and run the tests, run: 
```sh
make test
```

## Cleaning

To remove the object files, run: 
`make clean`

To remove the object files, executable, static libraries and the test disk image, run: 
`make pristine`# OS2Project10
