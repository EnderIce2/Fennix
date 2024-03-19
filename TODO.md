# TODO

---

- [x] Optimize SMP.
- [ ] Support IPv6.
- [ ] ~~Endianess of the network stack (currently: [HOST](LSB)<=>[NETWORK](MSB)). Not sure if this is a standard or not.~~ (???)
- [ ] Support 32-bit applications (ELF, PE, etc).
- [ ] ~~Do not map the entire memory. Map only the needed memory address at allocation time.~~ (we just copy the pages for userland, see `Fork()` inside [core/memory/page_table.cpp](core/memory/page_table.cpp))
- [ ] Implementation of logging (beside serial) with log rotation.
- [x] Implement a better task manager. (replace struct P/TCB with classes)
- [ ] Rewrite virtual file system.
- [ ] Colors in crash screen are not following the kernel color scheme.
- [x] ~~Find a way to add intrinsics.~~ (not feasible, use inline assembly)
- [ ] Rework PSF1 font loader.
- [x] ~~The cleanup should be done by a thread (tasking). This is done to avoid a deadlock.~~ (not needed, everything is done by the scheduler)
- [ ] Implement a better Display::SetBrightness() function.
- [ ] Fix memcpy, memset and memcmp functions (they are not working properly with SIMD).
- [ ] Fully support i386.
- [ ] Support Aarch64.
- [ ] ~~SMP trampoline shouldn't be hardcoded at 0x2000.~~ (0x2000 is in the conventional memory, it's fine)
- [ ] Rework the stack guard.
- [x] Mutex implementation.
- [ ] Update SMBIOS functions to support newer versions and actually use it.
- [x] COW (Copy On Write) for the virtual memory. (https://en.wikipedia.org/wiki/Copy-on-write)
- [x] Implement lazy allocation. (page faults)
- [ ] Bootstrap should have a separate bss section + PHDR.
- [ ] Reimplement the driver conflict detection.
- [x] Elf loader shouldn't create a full copy of the elf binary. Copy only the needed sections.
- [ ] Use NX-bit.
- [ ] Fix std::string.
- [x] Rewrite PS/2 drivers.
- [ ] Improve signal handling.

- [ ] Improve the way the kernel crashes.
	- Add panic() function.
	- Handle assertion failures.

- [ ] Optimize screen printing.
	- On real hardware it's very slow, a solution is dirty printing.

- [x] Thread ids should follow the POSIX standard.
	- When a new process is created, the first thread should have the same id as the process id.
	- If pid is 400, the first thread should have the id 400. The second thread should have the id 401, etc.

- [ ] Optimize the scheduler
	- Create a separate list for processes that are waiting for a resource or a signal, etc.
	- Use all cores to schedule threads.

- [x] Improve Remap() function.
	- Remove Unmap & Map logic. Remove all flags directly.
