# Core components

This directory contains the core components of the project. These components are used by the kernel to provide the basic functionality of the operating system.

---

## 💾 Memory

Contains the memory management code.
It is responsible for allocating and freeing memory.
It also provides the `kmalloc`, `kcalloc`, `krealloc` and `kfree` functions that are used by the rest of the kernel.
