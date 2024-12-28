# tools

---

In this directory, you will find:
- An error parser for qemu debug output.
- Ethernet packet reader for testing network connection.
- A Makefile script to clone, patch and build gcc, binutils and qemu for you.
- `website` directory containing the source code for the website.

## Reasons for the following patches

- `gcc.patch`
	- Required for cross-compiling the OS.

- `binutils-gdb.patch`
	- Same as above.

- `qemu.patch`
	- Removed patch "Replace GDB exit calls with proper shutdown" so when we stop debugging in vscode, the qemu process is automatically killed.

## Useful links

- [Create git patches](https://stackoverflow.com/a/15438863/9352057)
- [QEMU git tags](https://gitlab.com/qemu-project/qemu/-/tags)
