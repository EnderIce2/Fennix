# File System Implementation

---

## Nodes

### /storage

- `node.cpp`
	- **Node <=> device**
	- Handles open/close/read/write operations for the device and holds information about the file

<br>

- `reference.cpp`
	- **kernel/user <=> node.cpp**
	- Maintains the count of references to a node and the seek position

<br>

- `descriptor.cpp`
	- **user <=> reference.cpp**
	- Manages the file descriptor table for user processes

### /storage/fs

This directory contains the implementations of various file systems, such as `fat32.cpp` and `ustar.cpp`.

### /storage/devices

This directory houses implementations of various devices, including /dev/null, /dev/zero, /dev/random, and more.
