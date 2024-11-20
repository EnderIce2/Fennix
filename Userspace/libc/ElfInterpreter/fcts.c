#include "fcts.h"

#include "../../../Kernel/syscalls.h"
#include "../../../Kernel/ipc.h"

uintptr_t RequestPages(size_t Count)
{
	return syscall6(sc_mmap, NULL, Count * 0x1000,
					sc_PROT_READ | sc_PROT_WRITE,
					sc_MAP_ANONYMOUS | sc_MAP_PRIVATE,
					0, 0);
}

int FreePages(uintptr_t Address, size_t Count)
{
	return syscall2(sc_munmap, Address, Count * 0x1000);
}

int abs(int i) { return i < 0 ? -i : i; }

void swap(char *x, char *y)
{
	char t = *x;
	*x = *y;
	*y = t;
}

char *reverse(char *Buffer, int i, int j)
{
	while (i < j)
		swap(&Buffer[i++], &Buffer[j--]);
	return Buffer;
}

char *ltoa(long Value, char *Buffer, int Base)
{
	if (Base < 2 || Base > 32)
		return Buffer;

	long n = (long)abs((int)Value);
	int i = 0;

	while (n)
	{
		int r = n % Base;
		if (r >= 10)
			Buffer[i++] = 65 + (r - 10);
		else
			Buffer[i++] = 48 + r;
		n = n / Base;
	}

	if (i == 0)
		Buffer[i++] = '0';

	if (Value < 0 && Base == 10)
		Buffer[i++] = '-';

	Buffer[i] = '\0';
	return reverse(Buffer, 0, i - 1);
}

void PutCharToKernelConsole(char c)
{
	__asm__ __volatile__("syscall"
						 :
						 : "a"(1), "D"(c), "S"(0)
						 : "rcx", "r11", "memory");
}

void Print__(char *String)
{
	for (short i = 0; String[i] != '\0'; i++)
		PutCharToKernelConsole(String[i]);
}

void PrintNL__(char *String)
{
	Print__(String);
	Print__("\n");
}

void *memcpy(void *dest, const void *src, size_t n)
{
	uint8_t *d = dest;
	const uint8_t *s = src;
	while (n--)
		*d++ = *s++;
	return dest;
}

void *memset(void *s, int c, size_t n)
{
	uint8_t *p = s;
	while (n--)
		*p++ = c;
}

int strcmp(const char *l, const char *r)
{
	for (; *l == *r && *l; l++, r++)
		;
	return *(unsigned char *)l - *(unsigned char *)r;
}

struct Elf64_Dyn ELFGetDynamicTag(char *Path, enum DynamicTags Tag)
{
	int fd = syscall2(sc_open, Path, (long)"r");
	if (fd < 0)
		syscall1(sc_exit, -0xF17E);

	Elf64_Ehdr ELFHeader;
	syscall3(sc_read, fd, &ELFHeader, sizeof(Elf64_Ehdr));

	Elf64_Phdr ItrProgramHeader;
	for (Elf64_Half i = 0; i < ELFHeader.e_phnum; i++)
	{
		// memcpy(&ItrProgramHeader, (uint8_t *)ElfFile + ELFHeader.e_phoff + ELFHeader.e_phentsize * i, sizeof(Elf64_Phdr));
		syscall3(sc_lseek, fd, ELFHeader.e_phoff + ELFHeader.e_phentsize * i, sc_SEEK_SET);
		syscall3(sc_read, fd, &ItrProgramHeader, sizeof(Elf64_Phdr));
		if (ItrProgramHeader.p_type == PT_DYNAMIC)
		{
			struct Elf64_Dyn Dynamic; // = (struct Elf64_Dyn *)((uint8_t *)ElfFile + ItrProgramHeader.p_offset);
			syscall3(sc_lseek, fd, ItrProgramHeader.p_offset, sc_SEEK_SET);
			syscall3(sc_read, fd, &Dynamic, ItrProgramHeader.p_filesz);
			for (size_t i = 0; i < ItrProgramHeader.p_filesz / sizeof(struct Elf64_Dyn); i++)
			{
				if (Dynamic.d_tag == Tag || Dynamic.d_tag == DT_NULL)
				{
					syscall1(sc_close, fd);
					return Dynamic;
				}

				syscall3(sc_lseek, fd, ItrProgramHeader.p_offset + (i + 1) * sizeof(struct Elf64_Dyn), sc_SEEK_SET);
				syscall3(sc_read, fd, &Dynamic, sizeof(struct Elf64_Dyn));
			}
		}
	}
	syscall1(sc_close, fd);
	return (struct Elf64_Dyn){0};
}

Elf64_Shdr *GetELFSheader(Elf64_Ehdr *Header)
{
	Elf64_Off SheaderOffset = Header->e_shoff;
	return (Elf64_Shdr *)((uintptr_t)Header + SheaderOffset);
}

Elf64_Shdr *GetELFSection(Elf64_Ehdr *Header, uint64_t Index)
{
	Elf64_Shdr *Sheader = GetELFSheader(Header);
	return &Sheader[Index];
}

char *GetELFStringTable(Elf64_Ehdr *Header)
{
	if (Header->e_shstrndx == SHN_UNDEF)
		return NULL;
	return (char *)Header + GetELFSection(Header, Header->e_shstrndx)->sh_offset;
}

Elf64_Sym ELFLookupSymbol(char *Path, const char *Name)
{
	int fd = syscall2(sc_open, Path, (long)"r");
	if (fd < 0)
		syscall1(sc_exit, -0xF17E);

	Elf64_Ehdr ELFHeader;
	syscall3(sc_read, fd, &ELFHeader, sizeof(Elf64_Ehdr));

	Elf64_Shdr SymbolTable;
	Elf64_Shdr StringTable;
	Elf64_Sym Symbol;
	char *String = NULL;

	for (Elf64_Half i = 0; i < ELFHeader.e_shnum; i++)
	{
		Elf64_Shdr shdr;
		syscall3(sc_lseek, fd,
				 ELFHeader.e_shoff + ELFHeader.e_shentsize * i,
				 sc_SEEK_SET);
		syscall3(sc_read, fd, &shdr, sizeof(Elf64_Shdr));

		switch (shdr.sh_type)
		{
		case SHT_SYMTAB:
		{
			SymbolTable = shdr;
			syscall3(sc_lseek, fd,
					 ELFHeader.e_shoff + ELFHeader.e_shentsize * shdr.sh_link,
					 sc_SEEK_SET);
			syscall3(sc_read, fd, &StringTable, sizeof(Elf64_Shdr));
			break;
		}
		default:
		{
			break;
		}
		}
	}

	if (SymbolTable.sh_size == 0 || StringTable.sh_size == 0)
	{
		syscall1(sc_close, fd);
		return (Elf64_Sym){0};
	}

	for (size_t i = 0; i < (SymbolTable.sh_size / sizeof(Elf64_Sym)); i++)
	{
		// Symbol = (Elf64_Sym *)((uintptr_t)Header + SymbolTable->sh_offset + (i * sizeof(Elf64_Sym)));
		// String = (char *)((uintptr_t)Header + StringTable->sh_offset + Symbol->st_name);
		syscall3(sc_lseek, fd, SymbolTable.sh_offset + (i * sizeof(Elf64_Sym)), sc_SEEK_SET);
		syscall3(sc_read, fd, &Symbol, sizeof(Elf64_Sym));

		syscall3(sc_lseek, fd, StringTable.sh_offset + Symbol.st_name, sc_SEEK_SET);
		syscall3(sc_read, fd, &String, sizeof(char *));

		if (strcmp(String, Name) == 0)
		{
			syscall1(sc_close, fd);
			return Symbol;
		}
	}

	syscall1(sc_close, fd);
	return (Elf64_Sym){0};
}
