WOOT is yet another hobby operating system.

Makefiles are meant to be built under Linux. Image creation makefile rules use sudo, so you may want to edit your sudoers file accordingly or deal with it your way. You'll most probably need losetup, mount and umount. Most of the project uses clang as its default compiler.

To build, just run `make` in top directory. With a little bit of luck everything will just build. hdd.img file containg ext2 disk image should be created. Image file can be run in a VM or `dd`'d to hard disk and booted on real PC.

Previous versions of this project can be found [here](https://github.com/pvc988/woot).