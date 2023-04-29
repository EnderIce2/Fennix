#ifndef __FENNIX_KERNEL_ELF_H__
#define __FENNIX_KERNEL_ELF_H__

#include <types.h>

// https://wiki.osdev.org/ELF_Tutorial
// https://github.com/torvalds/linux/blob/master/include/uapi/linux/elf.h

/* 32-bit ELF base types. */
typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef int32_t Elf32_Sword;
typedef uint32_t Elf32_Word;

/* 64-bit ELF base types. */
typedef uint64_t Elf64_Addr;
typedef uint16_t Elf64_Half;
typedef int16_t Elf64_SHalf;
typedef uint64_t Elf64_Off;
typedef int32_t Elf64_Sword;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;

#define EI_NIDENT 16

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
    unsigned char e_ident[EI_NIDENT]; /* ELF "magic number" */
    Elf64_Half e_type;
    Elf64_Half e_machine;
    Elf64_Word e_version;
    Elf64_Addr e_entry; /* Entry point virtual address */
    Elf64_Off e_phoff;  /* Program header table file offset */
    Elf64_Off e_shoff;  /* Section header table file offset */
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
    Elf64_Word sh_name;       /* Section name, index in string tbl */
    Elf64_Word sh_type;       /* Type of section */
    Elf64_Xword sh_flags;     /* Miscellaneous section attributes */
    Elf64_Addr sh_addr;       /* Section virtual addr at execution */
    Elf64_Off sh_offset;      /* Section file offset */
    Elf64_Xword sh_size;      /* Size of section in bytes */
    Elf64_Word sh_link;       /* Index of another section */
    Elf64_Word sh_info;       /* Additional section information */
    Elf64_Xword sh_addralign; /* Section alignment */
    Elf64_Xword sh_entsize;   /* Entry size if section holds table */
} Elf64_Shdr;

typedef struct {
        Elf32_Word      p_type;
        Elf32_Off       p_offset;
        Elf32_Addr      p_vaddr;
        Elf32_Addr      p_paddr;
        Elf32_Word      p_filesz;
        Elf32_Word      p_memsz;
        Elf32_Word      p_flags;
        Elf32_Word      p_align;
} Elf32_Phdr;

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

typedef struct elf32_rel
{
    Elf32_Addr r_offset;
    Elf32_Word r_info;
} Elf32_Rel;

typedef struct elf64_rel
{
    Elf64_Addr r_offset; /* Location at which to apply the action */
    Elf64_Xword r_info;  /* index and type of relocation */
} Elf64_Rel;

typedef struct elf32_sym
{
    Elf32_Word st_name;
    Elf32_Addr st_value;
    Elf32_Word st_size;
    unsigned char st_info;
    unsigned char st_other;
    Elf32_Half st_shndx;
} Elf32_Sym;

typedef struct elf64_sym
{
    Elf64_Word st_name;     /* Symbol name, index in string tbl */
    unsigned char st_info;  /* Type and binding attributes */
    unsigned char st_other; /* No defined meaning, 0 */
    Elf64_Half st_shndx;    /* Associated section index */
    Elf64_Addr st_value;    /* Value of the symbol */
    Elf64_Xword st_size;    /* Associated symbol size */
} Elf64_Sym;

struct Elf32_Dyn
{
    Elf32_Sword d_tag; /* Type of dynamic table entry. */
    union
    {
        Elf32_Word d_val; /* Integer value of entry. */
        Elf32_Addr d_ptr; /* Pointer value of entry. */
    } d_un;
};

struct Elf64_Dyn
{
    Elf64_Sxword d_tag; /* Type of dynamic table entry. */
    union
    {
        Elf64_Xword d_val; /* Integer value of entry. */
        Elf64_Addr d_ptr;  /* Pointer value of entry. */
    } d_un;
};

typedef struct
{
    Elf64_Addr r_offset;
    Elf64_Xword r_info;
    Elf64_Sxword r_addend;
} Elf64_Rela;

enum Elf_Ident
{
    EI_MAG0 = 0,       // 0x7F
    EI_MAG1 = 1,       // 'E'
    EI_MAG2 = 2,       // 'L'
    EI_MAG3 = 3,       // 'F'
    EI_CLASS = 4,      // Architecture (32/64)
    EI_DATA = 5,       // Byte Order
    EI_VERSION = 6,    // ELF Version
    EI_OSABI = 7,      // OS Specific
    EI_ABIVERSION = 8, // OS Specific
    EI_PAD = 9         // Padding
};

enum Elf_OSABI
{
    ELFOSABI_NONE = 0,
    ELFOSABI_HPUX = 1,
    ELFOSABI_NETBSD = 2,
    ELFOSABI_LINUX = 3,
    ELFOSABI_HURD = 4,
    ELFOSABI_SOLARIS = 6,
    ELFOSABI_AIX = 7,
    ELFOSABI_IRIX = 8,
    ELFOSABI_FREEBSD = 9,
    ELFOSABI_TRU64 = 10,
    ELFOSABI_MODESTO = 11,
    ELFOSABI_OPENBSD = 12,
    ELFOSABI_OPENVMS = 13,
    ELFOSABI_NSK = 14,
    ELFOSABI_AROS = 15,
    ELFOSABI_FENIXOS = 16,
    ELFOSABI_CLOUDABI = 17,
    ELFOSABI_OPENVOS = 18,
    ELFOSABI_C6000_ELFABI = 64,
    ELFOSABI_C6000_LINUX = 65,
    ELFOSABI_ARM = 97,
    ELFOSABI_STANDALONE = 255
};

enum Elf_Type
{
    ET_NONE = 0,        // Unknown Type
    ET_REL = 1,         // Relocatable File
    ET_EXEC = 2,        // Executable File
    ET_DYN = 3,         // Shared Object File
    ET_CORE = 4,        // Core File
    ET_LOPROC = 0xff00, // Processor Specific
    ET_HIPROC = 0xffff  // Processor Specific
};

enum RtT_Types
{
    R_386_NONE = 0, // No relocation
    R_386_32 = 1,   // Symbol + Offset
    R_386_PC32 = 2, // Symbol + Offset - Section Offset

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

enum ProgFlags_Types
{
    PF_X = 1,
    PF_W = 2,
    PF_R = 4
};

enum StT_Bindings
{
    /**
     * @brief Local symbol. Symbol is not visible outside the object file.
     */
    STB_LOCAL = 0,
    /**
     * @brief Global symbol. These symbols are visible to all object files being combined.
     */
    STB_GLOBAL = 1,
    /**
     * @brief Weak symbols. These symbols are like global symbols, but their definitions are not required. Weak symbols are not visible outside the object file containing their definition.
     */
    STB_WEAK = 2,
    /**
     * @brief Values in this inclusive range are reserved for operating system-specific semantics.
     */
    STB_LOOS = 10,
    /**
     * @brief Values in this inclusive range are reserved for operating system-specific semantics.
     */
    STB_HIOS = 12,
    /**
     * @brief Values in this inclusive range are reserved for processor-specific semantics.
     */
    STB_LOPROC = 13,
    /**
     * @brief Values in this inclusive range are reserved for processor-specific semantics.
     */
    STB_HIPROC = 15
};

enum StT_Types
{
    STT_NOTYPE = 0, // No type
    STT_OBJECT = 1, // Variables, arrays, etc.
    STT_FUNC = 2    // Methods or functions
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

/* https://docs.oracle.com/cd/E19683-01/817-3677/chapter6-42444/index.html */
enum DynamicArrayTags
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

// used for Elf64_Sym st_info
#define ELF32_ST_BIND(info) ((info) >> 4)
#define ELF32_ST_TYPE(info) ((info)&0xf)
#define ELF32_ST_INFO(bind, type) (((bind) << 4) + ((type)&0xf))
#define ELF64_ST_BIND(info) ((info) >> 4)
#define ELF64_ST_TYPE(info) ((info)&0xf)
#define ELF64_ST_INFO(bind, type) (((bind) << 4) + ((type)&0xf))

// used for Elf64_Sym st_other
#define ELF32_ST_VISIBILITY(o) ((o)&0x3)
#define ELF64_ST_VISIBILITY(o) ((o)&0x3)

#define DO_386_32(S, A) ((S) + (A))
#define DO_386_PC32(S, A, P) ((S) + (A) - (P))

#define DO_64_64(S, A) ((S) + (A))
#define DO_64_PC32(S, A, P) ((S) + (A) - (P))

#define ELF32_R_SYM(i) ((i) >> 8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s, t) (((s) << 8) + (unsigned char)(t))

#define ELF64_R_SYM(i) ((i) >> 32)
#define ELF64_R_TYPE(i) ((i)&0xffffffffL)
#define ELF64_R_INFO(s, t) (((s) << 32) + ((t)&0xffffffffL))

#define SHN_UNDEF 0
#define SHN_ABS 0xfff1

#define SHT_NOBITS 8
#define SHT_REL 9

#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2

#define EM_386 0x3      // x86 Machine Type
#define EM_X86_64 0x3E  // 64bit
#define EM_ARM 0x28     // ARM
#define EM_AARCH64 0xb7 // ARM64

#define EV_CURRENT 0x1 // ELF Current Version

#define ELFMAG0 0x7F // e_ident[EI_MAG0]
#define ELFMAG1 'E'  // e_ident[EI_MAG1]
#define ELFMAG2 'L'  // e_ident[EI_MAG2]
#define ELFMAG3 'F'  // e_ident[EI_MAG3]

#define ELFDATANONE 0 /* e_ident[EI_DATA] */
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

#define ELFCLASSNONE 0 /* EI_CLASS */
#define ELFCLASS32 1
#define ELFCLASS64 2
#define ELFCLASSNUM 3

#define SHT_NULL 0                    /* Section header table entry unused */
#define SHT_PROGBITS 1                /* Program data */
#define SHT_SYMTAB 2                  /* Symbol table */
#define SHT_STRTAB 3                  /* String table */
#define SHT_RELA 4                    /* Relocation entries with addends */
#define SHT_HASH 5                    /* Symbol hash table */
#define SHT_DYNAMIC 6                 /* Dynamic linking information */
#define SHT_NOTE 7                    /* Notes */
#define SHT_NOBITS 8                  /* Program space with no data (bss) */
#define SHT_REL 9                     /* Relocation entries, no addends */
#define SHT_SHLIB 10                  /* Reserved */
#define SHT_DYNSYM 11                 /* Dynamic linker symbol table */
#define SHT_INIT_ARRAY 14             /* Array of constructors */
#define SHT_FINI_ARRAY 15             /* Array of destructors */
#define SHT_PREINIT_ARRAY 16          /* Array of pre-constructors */
#define SHT_GROUP 17                  /* Section group */
#define SHT_SYMTAB_SHNDX 18           /* Extended section indeces */
#define SHT_NUM 19                    /* Number of defined types.  */
#define SHT_LOOS 0x60000000           /* Start OS-specific.  */
#define SHT_GNU_ATTRIBUTES 0x6ffffff5 /* Object attributes.  */
#define SHT_GNU_HASH 0x6ffffff6       /* GNU-style hash table.  */
#define SHT_GNU_LIBLIST 0x6ffffff7    /* Prelink library list */
#define SHT_CHECKSUM 0x6ffffff8       /* Checksum for DSO content.  */
#define SHT_LOSUNW 0x6ffffffa         /* Sun-specific low bound.  */
#define SHT_SUNW_move 0x6ffffffa
#define SHT_SUNW_COMDAT 0x6ffffffb
#define SHT_SUNW_syminfo 0x6ffffffc
#define SHT_GNU_verdef 0x6ffffffd  /* Version definition section.  */
#define SHT_GNU_verneed 0x6ffffffe /* Version needs section.  */
#define SHT_GNU_versym 0x6fffffff  /* Version symbol table.  */
#define SHT_HISUNW 0x6fffffff      /* Sun-specific high bound.  */
#define SHT_HIOS 0x6fffffff        /* End OS-specific type */
#define SHT_LOPROC 0x70000000      /* Start of processor-specific */
#define SHT_HIPROC 0x7fffffff      /* End of processor-specific */
#define SHT_LOUSER 0x80000000      /* Start of application-specific */
#define SHT_HIUSER 0x8fffffff      /* End of application-specific */

#endif // !__FENNIX_KERNEL_ELF_H__
