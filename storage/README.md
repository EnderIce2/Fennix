# File System Implementation

---

## Nodes

### /storage

- `node.cpp`
	- **ref_node <=> device**
	- Handles open/close/read/write operations for the device

<br>

- `ref_node.cpp`
	- **kernel/user <=> node.cpp**
	- Maintains the count of references to a node and the seek position

<br>

- `file_descriptor.cpp`
	- **user <=> ref_node.cpp**
	- Manages the file descriptor table for user processes

<br>

- `kernel_io.cpp`
	- **kernel <=> file_descriptor.cpp**
	- Performs a similar role as `file_descriptor.cpp` but for kernel processes

### /storage/fs

This directory contains the implementations of various file systems, such as `fat32.cpp` and `ustar.cpp`.

### /storage/devices

This directory houses implementations of various devices, including /dev/null, /dev/zero, /dev/random, and more.
