#ifndef __FENNIX_LIB_ELF_LAZY_RESOLVE_H__
#define __FENNIX_LIB_ELF_LAZY_RESOLVE_H__

typedef __UINT32_TYPE__ Elf32_Addr;
typedef __UINT16_TYPE__ Elf32_Half;
typedef __UINT32_TYPE__ Elf32_Off;
typedef __INT32_TYPE__ Elf32_Sword;
typedef __UINT32_TYPE__ Elf32_Word;

typedef __UINT64_TYPE__ Elf64_Addr;
typedef __UINT16_TYPE__ Elf64_Half;
typedef __INT16_TYPE__ Elf64_SHalf;
typedef __UINT64_TYPE__ Elf64_Off;
typedef __INT32_TYPE__ Elf64_Sword;
typedef __UINT32_TYPE__ Elf64_Word;
typedef __UINT64_TYPE__ Elf64_Xword;
typedef __INT64_TYPE__ Elf64_Sxword;

enum SectionHeaderType
{
	SHT_NULL = 0,
	SHT_PROGBITS = 1,
	SHT_SYMTAB = 2,
	SHT_STRTAB = 3,
	SHT_RELA = 4,
	SHT_HASH = 5,
	SHT_DYNAMIC = 6,
	SHT_NOTE = 7,
	SHT_NOBITS = 8,
	SHT_REL = 9,
	SHT_SHLIB = 10,
	SHT_DYNSYM = 11,
	SHT_INIT_ARRAY = 14,
	SHT_FINI_ARRAY = 15,
	SHT_PREINIT_ARRAY = 16,
	SHT_GROUP = 17,
	SHT_SYMTAB_SHNDX = 18,
	SHT_NUM = 19,
	SHT_LOOS = 0x60000000,
	SHT_GNU_ATTRIBUTES = 0x6ffffff5,
	SHT_GNU_HASH = 0x6ffffff6,
	SHT_GNU_LIBLIST = 0x6ffffff7,
	SHT_CHECKSUM = 0x6ffffff8,
	SHT_LOSUNW = 0x6ffffffa,
	SHT_SUNW_move = 0x6ffffffa,
	SHT_SUNW_COMDAT = 0x6ffffffb,
	SHT_SUNW_syminfo = 0x6ffffffc,
	SHT_GNU_verdef = 0x6ffffffd,
	SHT_GNU_verneed = 0x6ffffffe,
	SHT_GNU_versym = 0x6fffffff,
	SHT_HISUNW = 0x6fffffff,
	SHT_HIOS = 0x6fffffff,
	SHT_LOPROC = 0x70000000,
	SHT_HIPROC = 0x7fffffff,
	SHT_LOUSER = 0x80000000,
	SHT_HIUSER = 0x8fffffff
};

#define ELF32_R_SYM(i) ((i) >> 8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s, t) (((s) << 8) + (unsigned char)(t))

#define ELF64_R_SYM(i) ((i) >> 32)
#define ELF64_R_TYPE(i) ((i)&0xffffffffL)
#define ELF64_R_INFO(s, t) (((s) << 32) + ((t)&0xffffffffL))

#define EI_NIDENT 16

#define SHN_UNDEF 0
#define SHN_ABS 0xfff1

enum DynamicTags
{
	DT_NULL = 0,
	DT_NEEDED = 1,
	DT_PLTRELSZ = 2,
	DT_PLTGOT = 3,
	DT_HASH = 4,
	DT_STRTAB = 5,
	DT_SYMTAB = 6,
	DT_RELA = 7,
	DT_RELASZ = 8,
	DT_RELAENT = 9,
	DT_STRSZ = 10,
	DT_SYMENT = 11,
	DT_INIT = 12,
	DT_FINI = 13,
	DT_SONAME = 14,
	DT_RPATH = 15,
	DT_SYMBOLIC = 16,
	DT_REL = 17,
	DT_RELSZ = 18,
	DT_RELENT = 19,
	DT_PLTREL = 20,
	DT_DEBUG = 21,
	DT_TEXTREL = 22,
	DT_JMPREL = 23,
	DT_BIND_NOW = 24,
	DT_INIT_ARRAY = 25,
	DT_FINI_ARRAY = 26,
	DT_INIT_ARRAYSZ = 27,
	DT_FINI_ARRAYSZ = 28,
	DT_RUNPATH = 29,
	DT_FLAGS = 30,
	DT_ENCODING = 32,
	DT_PREINIT_ARRAY = 32,
	DT_PREINIT_ARRAYSZ = 33,
	DT_LOOS = 0x6000000d,
	DT_SUNW_RTLDINF = 0x6000000e,
	DT_HIOS = 0x6ffff000,
	DT_VALRNGLO = 0x6ffffd00,
	DT_CHECKSUM = 0x6ffffdf8,
	DT_PLTPADSZ = 0x6ffffdf9,
	DT_MOVEENT = 0x6ffffdfa,
	DT_MOVESZ = 0x6ffffdfb,
	DT_FEATURE_1 = 0x6ffffdfc,
	DT_POSFLAG_1 = 0x6ffffdfd,
	DT_SYMINSZ = 0x6ffffdfe,
	DT_SYMINENT = 0x6ffffdff,
	DT_VALRNGHI = 0x6ffffdff,
	DT_ADDRRNGLO = 0x6ffffe00,
	DT_CONFIG = 0x6ffffefa,
	DT_DEPAUDIT = 0x6ffffefb,
	DT_AUDIT = 0x6ffffefc,
	DT_PLTPAD = 0x6ffffefd,
	DT_MOVETAB = 0x6ffffefe,
	DT_SYMINFO = 0x6ffffeff,
	DT_ADDRRNGHI = 0x6ffffeff,
	DT_RELACOUNT = 0x6ffffff9,
	DT_RELCOUNT = 0x6ffffffa,
	DT_FLAGS_1 = 0x6ffffffb,
	DT_VERDEF = 0x6ffffffc,
	DT_VERDEFNUM = 0x6ffffffd,
	DT_VERNEED = 0x6ffffffe,
	DT_VERNEEDNUM = 0x6fffffff,
	DT_LOPROC = 0x70000000,
	DT_SPARC_REGISTER = 0x70000001,
	DT_AUXILIARY = 0x7ffffffd,
	DT_USED = 0x7ffffffe,
	DT_FILTER = 0x7fffffff,
	DT_HIPROC = 0x7fffffff
};

enum SegmentTypes
{
	PT_NULL = 0,
	PT_LOAD = 1,
	PT_DYNAMIC = 2,
	PT_INTERP = 3,
	PT_NOTE = 4,
	PT_SHLIB = 5,
	PT_PHDR = 6,
	PT_TLS = 7,
	PT_LOSUNW = 0x6ffffffa,
	PT_SUNWBSS = 0x6ffffffb,
	PT_SUNWSTACK = 0x6ffffffa,
	PT_HISUNW = 0x6fffffff,
	PT_LOPROC = 0x70000000,
	PT_HIPROC = 0x7fffffff
};

enum RtT_Types
{
	R_386_NONE = 0,
	R_386_32 = 1,
	R_386_PC32 = 2,

	R_X86_64_NONE = 0,
	R_X86_64_64 = 1,
	R_X86_64_PC32 = 2,
	R_X86_64_GOT32 = 3,
	R_X86_64_PLT32 = 4,
	R_X86_64_COPY = 5,
	R_X86_64_GLOB_DAT = 6,
	R_X86_64_JUMP_SLOT = 7,
	R_X86_64_RELATIVE = 8,
	R_X86_64_GOTPCREL = 9,
	R_X86_64_32 = 10,
	R_X86_64_32S = 11,
	R_X86_64_16 = 12,
};

typedef struct elf32_hdr
{
	unsigned char e_ident[EI_NIDENT];
	Elf32_Half e_type;
	Elf32_Half e_machine;
	Elf32_Word e_version;
	Elf32_Addr e_entry;
	Elf32_Off e_phoff;
	Elf32_Off e_shoff;
	Elf32_Word e_flags;
	Elf32_Half e_ehsize;
	Elf32_Half e_phentsize;
	Elf32_Half e_phnum;
	Elf32_Half e_shentsize;
	Elf32_Half e_shnum;
	Elf32_Half e_shstrndx;
} Elf32_Ehdr;

typedef struct elf64_hdr
{
	unsigned char e_ident[EI_NIDENT];
	Elf64_Half e_type;
	Elf64_Half e_machine;
	Elf64_Word e_version;
	Elf64_Addr e_entry;
	Elf64_Off e_phoff;
	Elf64_Off e_shoff;
	Elf64_Word e_flags;
	Elf64_Half e_ehsize;
	Elf64_Half e_phentsize;
	Elf64_Half e_phnum;
	Elf64_Half e_shentsize;
	Elf64_Half e_shnum;
	Elf64_Half e_shstrndx;
} Elf64_Ehdr;

typedef struct elf32_shdr
{
	Elf32_Word sh_name;
	Elf32_Word sh_type;
	Elf32_Word sh_flags;
	Elf32_Addr sh_addr;
	Elf32_Off sh_offset;
	Elf32_Word sh_size;
	Elf32_Word sh_link;
	Elf32_Word sh_info;
	Elf32_Word sh_addralign;
	Elf32_Word sh_entsize;
} Elf32_Shdr;

typedef struct elf64_shdr
{
	Elf64_Word sh_name;
	Elf64_Word sh_type;
	Elf64_Xword sh_flags;
	Elf64_Addr sh_addr;
	Elf64_Off sh_offset;
	Elf64_Xword sh_size;
	Elf64_Word sh_link;
	Elf64_Word sh_info;
	Elf64_Xword sh_addralign;
	Elf64_Xword sh_entsize;
} Elf64_Shdr;

struct Elf32_Dyn
{
	Elf32_Sword d_tag;
	union
	{
		Elf32_Word d_val;
		Elf32_Addr d_ptr;
	} d_un;
};

struct Elf64_Dyn
{
	Elf64_Sxword d_tag;
	union
	{
		Elf64_Xword d_val;
		Elf64_Addr d_ptr;
	} d_un;
};

typedef struct
{
	Elf64_Word p_type;
	Elf64_Word p_flags;
	Elf64_Off p_offset;
	Elf64_Addr p_vaddr;
	Elf64_Addr p_paddr;
	Elf64_Xword p_filesz;
	Elf64_Xword p_memsz;
	Elf64_Xword p_align;
} Elf64_Phdr;

typedef struct
{
	Elf64_Addr r_offset;
	Elf64_Xword r_info;
	Elf64_Sxword r_addend;
} Elf64_Rela;

typedef struct elf64_sym
{
	Elf64_Word st_name;
	unsigned char st_info;
	unsigned char st_other;
	Elf64_Half st_shndx;
	Elf64_Addr st_value;
	Elf64_Xword st_size;
} Elf64_Sym;

#endif // !__FENNIX_LIB_ELF_LAZY_RESOLVE_H__
