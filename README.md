WOOT is yet another hobby operating system.

Makefiles are meant to be built under Linux. Image creation makefile rules use sudo, so you may want to edit your sudoers file accordingly or deal with it your way. You'll most probably need losetup, mount and umount. Most of the project uses clang as its default compiler.

To build, just run `make` in top directory. With a little bit of luck everything will just build. hdd.img file containg ext2 disk image should be created. Image file can be run in a VM or `dd`'d to hard disk and booted on real PC.

This version is not very interactive for now, since  the main goal of this version (0.3) is to move  window manager and most of the input stuff to userspace. To do that, somewhat stable userspace environment needs to be implemented first. So don't expect Quake II to be running that soon :)

Previous versions of this project can be found [here](https://github.com/pvc988/woot).