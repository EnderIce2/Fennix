#ifndef __FENNIX_LIBC_FUNCTIONS_H__
#define __FENNIX_LIBC_FUNCTIONS_H__

#include <types.h>
#include "elf.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

uintptr_t RequestPages(size_t Count);
int FreePages(uintptr_t Address, size_t Count);
int IPC(int Command, int Type, int ID, int Flags, void *Buffer, size_t Size);
uintptr_t KernelCTL(int Command, uint64_t Arg1, uint64_t Arg2, uint64_t Arg3, uint64_t Arg4);
int abs(int i);
void swap(char *x, char *y);
char *reverse(char *Buffer, int i, int j);
char *ltoa(long Value, char *Buffer, int Base);
void PutCharToKernelConsole(char c);
void Print__(char *String);
void PrintNL__(char *String);
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
int strcmp(const char *l, const char *r);

struct Elf64_Dyn ELFGetDynamicTag(char *Path, enum DynamicTags Tag);
Elf64_Shdr *GetELFSheader(Elf64_Ehdr *Header);
Elf64_Shdr *GetELFSection(Elf64_Ehdr *Header, uint64_t Index);
char *GetELFStringTable(Elf64_Ehdr *Header);
Elf64_Sym ELFLookupSymbol(char *Path, const char *Name);

#endif // !__FENNIX_LIBC_FUNCTIONS_H__
