/*
	This file is part of Fennix Drivers.

	Fennix Drivers is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Drivers is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Drivers. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __FENNIX_API_BASE_H__
#define __FENNIX_API_BASE_H__

#include <driver.h>

#define PAGE_SIZE 0x1000

typedef int CriticalState;

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

	/**
	 * @brief Allocate an amount of memory pages
	 * @param Pages The amount of pages to allocate
	 * @return A pointer to the allocated memory, NULL if failed
	 */
	void *AllocateMemory(size_t Pages);

	/**
	 * @brief Free an amount of memory pages
	 * @param Pointer The pointer to the memory to free
	 * @param Pages The amount of pages to free
	 *
	 * @note If the pointer is invalid, the error is ignored and the
	 * function returns
	 */
	void FreeMemory(void *Pointer, size_t Pages);

	/**
	 * @brief Append a flag to a page
	 * @param Address The address of the page
	 * @param Flag The flag to append
	 */
	void AppendMapFlag(void *Address, PageMapFlags Flag);

	/**
	 * @brief Remove a flag from a page
	 * @param Address The address of the page
	 * @param Flag The flag to remove
	 */
	void RemoveMapFlag(void *Address, PageMapFlags Flag);

	/**
	 * @brief Map pages
	 * @param PhysicalAddress The physical address to map
	 * @param VirtualAddress The virtual address to map to
	 * @param Pages The amount of pages to map
	 * @param Flags The flags to map the pages with
	 */
	void MapPages(void *PhysicalAddress, void *VirtualAddress, size_t Pages, uint32_t Flags);

	/**
	 * @brief Unmap pages
	 * @param VirtualAddress The virtual address to unmap
	 * @param Pages The amount of pages to unmap
	 */
	void UnmapPages(void *VirtualAddress, size_t Pages);

	/**
	 * @brief Print to the kernel terminal
	 * @param Format The format string
	 * @param ... The arguments to the format string
	 *
	 * @note The newline character is automatically appended
	 */
	void KernelPrint(const char *Format, ...);

	/**
	 * @brief Print to the kernel logger
	 * @param Format The format string
	 * @param ... The arguments to the format string
	 */
	void KernelLog(const char *Format, ...);

	/**
	 * @brief Register an interrupt handler
	 * @param IRQ The IRQ to register the handler for (IRQ0 != 0x32 but 0)
	 * @param Handler Function pointer to the handler
	 * @return 0 on success, errno on failure
	 */
	int RegisterInterruptHandler(uint8_t IRQ, void *Handler);

	/**
	 * @brief Override an interrupt handler
	 *
	 * This function will check all registered handlers (by the drivers)
	 * for the given IRQ and remove them. Then it will register the new
	 * handler.
	 *
	 * @param IRQ The IRQ to override the handler for (IRQ0 != 0x32 but 0)
	 * @param Handler Function pointer to the handler
	 * @return 0 on success, errno on failure
	 */
	int OverrideInterruptHandler(uint8_t IRQ, void *Handler);

	/**
	 * @brief Unregister an interrupt handler
	 * @param IRQ The IRQ to unregister the handler for (IRQ0 != 0x32 but 0)
	 * @param Handler Function pointer to the handler
	 * @return 0 on success, errno on failure
	 */
	int UnregisterInterruptHandler(uint8_t IRQ, void *Handler);

	/**
	 * @brief Unregister all interrupt handlers for a given handler
	 * @param Handler Function pointer to the handler
	 * @return 0 on success, errno on failure
	 */
	int UnregisterAllInterruptHandlers(void *Handler);

	/**
	 * @brief Copy memory
	 * @param Destination The destination
	 * @param Source The source
	 * @param Length The length of the memory to copy
	 * @return The destination
	 */
	void *MemoryCopy(void *Destination, const void *Source, size_t Length);

	/**
	 * @brief Set memory
	 * @param Destination The destination
	 * @param Value The value to set
	 * @param Length The length of the memory to set
	 * @return The destination
	 */
	void *MemorySet(void *Destination, int Value, size_t Length);

	/**
	 * @brief Move memory
	 * @param Destination The destination
	 * @param Source The source
	 * @param Length The length of the memory to move
	 * @return The destination
	 */
	void *MemoryMove(void *Destination, const void *Source, size_t Length);

	/**
	 * @brief String length
	 * @param String The string
	 * @return The length of the string
	 */
	size_t StringLength(const char String[]);

	char *strstr(const char *Haystack, const char *Needle);

	/**
	 * @brief Create a kernel process
	 * @param Name The name of the process
	 * @return The PID of the process, this function never fails
	 */
	pid_t CreateKernelProcess(const char *Name);

	/**
	 * @brief Create a kernel thread
	 * @param pId The PID of the process to create the thread in
	 * @param Name The name of the thread
	 * @param EntryPoint The entry point of the thread
	 * @param Argument The argument to pass to the thread (rdi)
	 * @return The TID of the thread, this function never fails
	 */
	pid_t CreateKernelThread(pid_t pId, const char *Name, void *EntryPoint,
							 void *Argument);

	/**
	 * @brief Get the PID of the current process
	 * @return The PID of the current process
	 */
	pid_t GetCurrentProcess();

	/**
	 * @brief Kill a process
	 * @param pId The PID of the process to kill
	 * @param ExitCode The exit code of the process
	 * @return 0 on success, errno on failure
	 */
	int KillProcess(pid_t pId, int ExitCode);

	/**
	 * @brief Kill a thread
	 * @param tId The TID of the thread to kill
	 * @param pId The PID of the process the thread is in
	 * @param ExitCode The exit code of the thread
	 * @return 0 on success, errno on failure
	 */
	int KillThread(pid_t tId, pid_t pId, int ExitCode);

	/**
	 * @brief Yield the current thread
	 *
	 * This function will yield the current thread to the scheduler.
	 */
	void Yield();

	/**
	 * @brief Sleep for a given amount of milliseconds
	 * @param Milliseconds The amount of milliseconds to sleep
	 */
	void Sleep(uint64_t Milliseconds);

	/**
	 * @brief Enter a critical section
	 * @return The previous interrupt state
	 *
	 * This function will disable interrupts and return the previous
	 * interrupt state.
	 */
	CriticalState EnterCriticalSection();

	/**
	 * @brief Leave a critical section
	 * @param PreviousState The previous interrupt state
	 *
	 * This function will restore the previous interrupt state.
	 */
	void LeaveCriticalSection(CriticalState PreviousState);

	/** @copydoc EnterCriticalSection */
#define ECS CriticalState __ECS = EnterCriticalSection()

	/** @copydoc LeaveCriticalSection */
#define LCS LeaveCriticalSection(__ECS)

#define DriverInfo(_Name, _Description, _Author, _MajorVersion,  \
				   _MinorVersion, _PathVersion, _License)        \
	__attribute__((section(".driver.info"))) struct __DriverInfo \
		__di = {.Name = _Name,                                   \
				.Description = _Description,                     \
				.Author = _Author,                               \
				.Version = {.APIVersion = 0,                     \
							.Major = _MajorVersion,              \
							.Minor = _MinorVersion,              \
							.Patch = _PathVersion},              \
				.License = _License}

#ifdef DEBUG
#define DebugLog(m, ...) KernelLog(m, ##__VA_ARGS__)
#else
#define DebugLog(m, ...)
#endif // DEBUG

#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef __cplusplus
namespace std
{
	typedef decltype(sizeof(0)) size_t;
}

void *operator new(std::size_t Size);
void *operator new[](std::size_t Size);
void operator delete(void *Pointer);
void operator delete[](void *Pointer);
void operator delete(void *Pointer, std::size_t Size);
void operator delete[](void *Pointer, std::size_t Size);

#endif // __cplusplus

#define memcpy(dest, src, len) MemoryCopy(dest, src, len)
#define memset(dest, val, len) MemorySet(dest, val, len)
#define memmove(dest, src, len) MemoryMove(dest, src, len)

#define strlen(str) StringLength(str)

#endif // !__FENNIX_API_BASE_H__
