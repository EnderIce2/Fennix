/*
	This file is part of Fennix Kernel.

	Fennix Kernel is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Kernel is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __FENNIX_KERNEL_ELF_H__
#define __FENNIX_KERNEL_ELF_H__

#include <types.h>

/* 32-bit ELF base types. */
typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef int32_t Elf32_Sword;
typedef uint32_t Elf32_Word;
typedef Elf32_Sword Elf32_pid_t;

/* 64-bit ELF base types. */
typedef uint64_t Elf64_Addr;
typedef uint16_t Elf64_Half;
typedef int16_t Elf64_SHalf;
typedef uint64_t Elf64_Off;
typedef int32_t Elf64_Sword;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;
typedef Elf64_Sword Elf64_pid_t;

#define ELF_NGREG 23
typedef Elf32_Word Elf32_greg_t[ELF_NGREG];
typedef Elf64_Xword Elf64_greg_t[ELF_NGREG];

enum IdentificationIndex
{
	EI_MAG0 = 0,
	EI_MAG1 = 1,
	EI_MAG2 = 2,
	EI_MAG3 = 3,
	EI_CLASS = 4,
	EI_DATA = 5,
	EI_VERSION = 6,
	EI_OSABI = 7,
	EI_ABIVERSION = 8,
	EI_PAD = 9,
	EI_NIDENT = 16
};

enum Elf_OSABI
{
	ELFOSABI_NONE = 0,
	ELFOSABI_SYSV = 0,
	ELFOSABI_HPUX = 1,
	ELFOSABI_NETBSD = 2,
	ELFOSABI_GNU = 3,
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

enum FileIdentifiers
{
	ET_NONE = 0,
	ET_REL = 1,
	ET_EXEC = 2,
	ET_DYN = 3,
	ET_CORE = 4,
	ET_LOPROC = 0xff00,
	ET_HIPROC = 0xffff
};

enum RelocationTypes
{
	R_386_NONE = 0,
	R_386_32 = 1,
	R_386_PC32 = 2,
	R_386_GOT32 = 3,
	R_386_PLT32 = 4,
	R_386_COPY = 5,
	R_386_GLOB_DAT = 6,
	R_386_JMP_SLOT = 7,
	R_386_RELATIVE = 8,
	R_386_GOTOFF = 9,
	R_386_GOTPC = 10,
	R_386_32PLT = 11,
	R_386_TLS_TPOFF = 14,
	R_386_TLS_IE = 15,
	R_386_TLS_GOTIE = 16,
	R_386_TLS_LE = 17,
	R_386_TLS_GD = 18,
	R_386_TLS_LDM = 19,
	R_386_16 = 20,
	R_386_PC16 = 21,
	R_386_8 = 22,
	R_386_PC8 = 23,
	R_386_TLS_GD_32 = 24,
	R_386_TLS_GD_PUSH = 25,
	R_386_TLS_GD_CALL = 26,
	R_386_TLS_GD_POP = 27,
	R_386_TLS_LDM_32 = 28,
	R_386_TLS_LDM_PUSH = 29,
	R_386_TLS_LDM_CALL = 30,
	R_386_TLS_LDM_POP = 31,
	R_386_TLS_LDO_32 = 32,
	R_386_TLS_IE_32 = 33,
	R_386_TLS_LE_32 = 34,
	R_386_TLS_DTPMOD32 = 35,
	R_386_TLS_DTPOFF32 = 36,
	R_386_TLS_TPOFF32 = 37,
	R_386_SIZE32 = 38,
	R_386_TLS_GOTDESC = 39,
	R_386_TLS_DESC_CALL = 40,
	R_386_TLS_DESC = 41,
	R_386_IRELATIVE = 42,
	R_386_GOT32X = 43,
	R_386_NUM = 44,

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
	R_X86_64_PC16 = 13,
	R_X86_64_8 = 14,
	R_X86_64_PC8 = 15,
	R_X86_64_DTPMOD64 = 16,
	R_X86_64_DTPOFF64 = 17,
	R_X86_64_TPOFF64 = 18,
	R_X86_64_TLSGD = 19,
	R_X86_64_TLSLD = 20,
	R_X86_64_DTPOFF32 = 21,
	R_X86_64_GOTTPOFF = 22,
	R_X86_64_TPOFF32 = 23,
	R_X86_64_PC64 = 24,
	R_X86_64_GOTOFF64 = 25,
	R_X86_64_GOTPC32 = 26,
	R_X86_64_GOT64 = 27,
	R_X86_64_GOTPCREL64 = 28,
	R_X86_64_GOTPC64 = 29,
	R_X86_64_GOTPLT64 = 30,
	R_X86_64_PLTOFF64 = 31,
	R_X86_64_SIZE32 = 32,
	R_X86_64_SIZE64 = 33,
	R_X86_64_GOTPC32_TLSDESC = 34,
	R_X86_64_TLSDESC_CALL = 35,
	R_X86_64_TLSDESC = 36,
	R_X86_64_IRELATIVE = 37,
	R_X86_64_RELATIVE64 = 38,
	R_X86_64_GOTPCRELX = 41,
	R_X86_64_REX_GOTPCRELX = 42,
	R_X86_64_NUM = 43
};

enum SegmentFlags
{
	PF_X = 1,
	PF_W = 2,
	PF_R = 4,
	PF_MASKPROC = 0xf0000000
};

enum SymbolBindings
{
	STB_LOCAL = 0,
	STB_GLOBAL = 1,
	STB_WEAK = 2,
	STB_LOOS = 10,
	STB_HIOS = 12,
	STB_LOPROC = 13,
	STB_HIPROC = 15
};

enum SymbolTypes
{
	STT_NOTYPE = 0,
	STT_OBJECT = 1,
	STT_FUNC = 2,
	STT_SECTION = 3,
	STT_FILE = 4,
	STT_COMMON = 5,
	STT_LOOS = 10,
	STT_HIOS = 12,
	STT_LOPROC = 13,
	STT_SPARC_REGISTER = 13,
	STT_HIPROC = 15
};

#define STN_UNDEF 0

enum SymbolVisibilities
{
	STV_DEFAULT = 0,
	STV_INTERNAL = 1,
	STV_HIDDEN = 2,
	STV_PROTECTED = 3
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
	PT_LOOS = 0x60000000, /* OS-specific */
	PT_HIOS = 0x6fffffff, /* OS-specific */
	PT_LOPROC = 0x70000000,
	PT_HIPROC = 0x7fffffff,
	PT_GNU_EH_FRAME = (PT_LOOS + 0x474e550),
	PT_GNU_STACK = (PT_LOOS + 0x474e551),
	PT_GNU_RELRO = (PT_LOOS + 0x474e552),
	PT_GNU_PROPERTY = (PT_LOOS + 0x474e553)
};

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

/* Used for Elf64_Sym st_info */
#define ELF32_ST_BIND(info) ((info) >> 4)
#define ELF32_ST_TYPE(info) ((info) & 0xf)
#define ELF32_ST_INFO(bind, type) (((bind) << 4) + ((type) & 0xf))

#define ELF64_ST_BIND(info) ((info) >> 4)
#define ELF64_ST_TYPE(info) ((info) & 0xf)
#define ELF64_ST_INFO(bind, type) (((bind) << 4) + ((type) & 0xf))

/* Used for Elf64_Sym st_other */
#define ELF32_ST_VISIBILITY(o) ((o) & 0x3)
#define ELF64_ST_VISIBILITY(o) ((o) & 0x3)

#define DO_386_32(S, A) ((S) + (A))
#define DO_386_PC32(S, A, P) ((S) + (A) - (P))

#define DO_64_64(S, A) ((S) + (A))
#define DO_64_PC32(S, A, P) ((S) + (A) - (P))

#define ELF32_R_SYM(i) ((i) >> 8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s, t) (((s) << 8) + (unsigned char)(t))

#define ELF64_R_SYM(i) ((i) >> 32)
#define ELF64_R_TYPE(i) ((i) & 0xffffffffL)
#define ELF64_R_INFO(s, t) (((s) << 32) + ((t) & 0xffffffffL))

#define SHN_UNDEF 0
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC 0xff00
#define SHN_BEFORE 0xff00
#define SHN_AFTER 0xff01
#define SHN_HIPROC 0xff1f
#define SHN_ABS 0xfff1
#define SHN_COMMON 0xfff2
#define SHN_HIRESERVE 0xffff

#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MERGE 0x10
#define SHF_STRINGS 0x20
#define SHF_INFO_LINK 0x40
#define SHF_LINK_ORDER 0x80
#define SHF_OS_NONCONFORMING 0x100
#define SHF_GROUP 0x200
#define SHF_TLS 0x400
#define SHF_MASKOS 0x0ff00000
#define SHF_ORDERED 0x40000000
#define SHF_EXCLUDE 0x80000000
#define SHF_MASKPROC 0xf0000000

#define EM_NONE 0			 /*	No machine */
#define EM_M32 1			 /*	AT&T WE 32100 */
#define EM_SPARC 2			 /*	SPARC */
#define EM_386 3			 /*	Intel 80386 */
#define EM_68K 4			 /*	Motorola 68000 */
#define EM_88K 5			 /*	Motorola 88000 */
#define EM_IAMCU 6			 /*	Intel MCU */
#define EM_860 7			 /*	Intel 80860 */
#define EM_MIPS 8			 /*	MIPS I Architecture */
#define EM_S370 9			 /*	IBM System/370 Processor */
#define EM_MIPS_RS3_LE 10	 /*	MIPS RS3000 Little-endian */
#define EM_PARISC 15		 /*	Hewlett-Packard PA-RISC */
#define EM_VPP500 17		 /*	Fujitsu VPP500 */
#define EM_SPARC32PLUS 18	 /*	Enhanced instruction set SPARC */
#define EM_960 19			 /*	Intel 80960 */
#define EM_PPC 20			 /*	PowerPC */
#define EM_PPC64 21			 /*	64-bit PowerPC */
#define EM_S390 22			 /*	IBM System/390 Processor */
#define EM_SPU 23			 /*	IBM SPU/SPC */
#define EM_V800 36			 /*	NEC V800 */
#define EM_FR20 37			 /*	Fujitsu FR20 */
#define EM_RH32 38			 /*	TRW RH-32 */
#define EM_RCE 39			 /*	Motorola RCE */
#define EM_ARM 40			 /*	ARM 32-bit architecture (AARCH32) */
#define EM_ALPHA 41			 /*	Digital Alpha */
#define EM_SH 42			 /*	Hitachi SH */
#define EM_SPARCV9 43		 /*	SPARC Version 9 */
#define EM_TRICORE 44		 /*	Siemens TriCore embedded processor */
#define EM_ARC 45			 /*	Argonaut RISC Core, Argonaut Technologies Inc. */
#define EM_H8_300 46		 /*	Hitachi H8/300 */
#define EM_H8_300H 47		 /*	Hitachi H8/300H */
#define EM_H8S 48			 /*	Hitachi H8S */
#define EM_H8_500 49		 /*	Hitachi H8/500 */
#define EM_IA_64 50			 /*	Intel IA-64 processor architecture */
#define EM_MIPS_X 51		 /*	Stanford MIPS-X */
#define EM_COLDFIRE 52		 /*	Motorola ColdFire */
#define EM_68HC12 53		 /*	Motorola M68HC12 */
#define EM_MMA 54			 /*	Fujitsu MMA Multimedia Accelerator */
#define EM_PCP 55			 /*	Siemens PCP */
#define EM_NCPU 56			 /*	Sony nCPU embedded RISC processor */
#define EM_NDR1 57			 /*	Denso NDR1 microprocessor */
#define EM_STARCORE 58		 /*	Motorola Star*Core processor */
#define EM_ME16 59			 /*	Toyota ME16 processor */
#define EM_ST100 60			 /*	STMicroelectronics ST100 processor */
#define EM_TINYJ 61			 /*	Advanced Logic Corp. TinyJ embedded processor family */
#define EM_X86_64 62		 /*	AMD x86-64 architecture */
#define EM_PDSP 63			 /*	Sony DSP Processor */
#define EM_PDP10 64			 /*	Digital Equipment Corp. PDP-10 */
#define EM_PDP11 65			 /*	Digital Equipment Corp. PDP-11 */
#define EM_FX66 66			 /*	Siemens FX66 microcontroller */
#define EM_ST9PLUS 67		 /*	STMicroelectronics ST9+ 8/16 bit microcontroller */
#define EM_ST7 68			 /*	STMicroelectronics ST7 8-bit microcontroller */
#define EM_68HC16 69		 /*	Motorola MC68HC16 Microcontroller */
#define EM_68HC11 70		 /*	Motorola MC68HC11 Microcontroller */
#define EM_68HC08 71		 /*	Motorola MC68HC08 Microcontroller */
#define EM_68HC05 72		 /*	Motorola MC68HC05 Microcontroller */
#define EM_SVX 73			 /*	Silicon Graphics SVx */
#define EM_ST19 74			 /*	STMicroelectronics ST19 8-bit microcontroller */
#define EM_VAX 75			 /*	Digital VAX */
#define EM_CRIS 76			 /*	Axis Communications 32-bit embedded processor */
#define EM_JAVELIN 77		 /*	Infineon Technologies 32-bit embedded processor */
#define EM_FIREPATH 78		 /*	Element 14 64-bit DSP Processor */
#define EM_ZSP 79			 /*	LSI Logic 16-bit DSP Processor */
#define EM_MMIX 80			 /*	Donald Knuth's educational 64-bit processor */
#define EM_HUANY 81			 /*	Harvard University machine-independent object files */
#define EM_PRISM 82			 /*	SiTera Prism */
#define EM_AVR 83			 /*	Atmel AVR 8-bit microcontroller */
#define EM_FR30 84			 /*	Fujitsu FR30 */
#define EM_D10V 85			 /*	Mitsubishi D10V */
#define EM_D30V 86			 /*	Mitsubishi D30V */
#define EM_V850 87			 /*	NEC v850 */
#define EM_M32R 88			 /*	Mitsubishi M32R */
#define EM_MN10300 89		 /*	Matsushita MN10300 */
#define EM_MN10200 90		 /*	Matsushita MN10200 */
#define EM_PJ 91			 /*	picoJava */
#define EM_OPENRISC 92		 /*	OpenRISC 32-bit embedded processor */
#define EM_ARC_COMPACT 93	 /*	ARC International ARCompact processor (old spelling/synonym: EM_ARC_A5) */
#define EM_XTENSA 94		 /*	Tensilica Xtensa Architecture */
#define EM_VIDEOCORE 95		 /*	Alphamosaic VideoCore processor */
#define EM_TMM_GPP 96		 /*	Thompson Multimedia General Purpose Processor */
#define EM_NS32K 97			 /*	National Semiconductor 32000 series */
#define EM_TPC 98			 /*	Tenor Network TPC processor */
#define EM_SNP1K 99			 /*	Trebia SNP 1000 processor */
#define EM_ST200 100		 /*	STMicroelectronics (www.st.com) ST200 microcontroller */
#define EM_IP2K 101			 /*	Ubicom IP2xxx microcontroller family */
#define EM_MAX 102			 /*	MAX Processor */
#define EM_CR 103			 /*	National Semiconductor CompactRISC microprocessor */
#define EM_F2MC16 104		 /*	Fujitsu F2MC16 */
#define EM_MSP430 105		 /*	Texas Instruments embedded microcontroller msp430 */
#define EM_BLACKFIN 106		 /*	Analog Devices Blackfin (DSP) processor */
#define EM_SE_C33 107		 /*	S1C33 Family of Seiko Epson processors */
#define EM_SEP 108			 /*	Sharp embedded microprocessor */
#define EM_ARCA 109			 /*	Arca RISC Microprocessor */
#define EM_UNICORE 110		 /*	Microprocessor series from PKU-Unity Ltd. and MPRC of Peking University */
#define EM_EXCESS 111		 /*	eXcess: 16/32/64-bit configurable embedded CPU */
#define EM_DXP 112			 /*	Icera Semiconductor Inc. Deep Execution Processor */
#define EM_ALTERA_NIOS2 113	 /*	Altera Nios II soft-core processor */
#define EM_CRX 114			 /*	National Semiconductor CompactRISC CRX microprocessor */
#define EM_XGATE 115		 /*	Motorola XGATE embedded processor */
#define EM_C166 116			 /*	Infineon C16x/XC16x processor */
#define EM_M16C 117			 /*	Renesas M16C series microprocessors */
#define EM_DSPIC30F 118		 /*	Microchip Technology dsPIC30F Digital Signal Controller */
#define EM_CE 119			 /*	Freescale Communication Engine RISC core */
#define EM_M32C 120			 /*	Renesas M32C series microprocessors */
#define EM_TSK3000 131		 /*	Altium TSK3000 core */
#define EM_RS08 132			 /*	Freescale RS08 embedded processor */
#define EM_SHARC 133		 /*	Analog Devices SHARC family of 32-bit DSP processors */
#define EM_ECOG2 134		 /*	Cyan Technology eCOG2 microprocessor */
#define EM_SCORE7 135		 /*	Sunplus S+core7 RISC processor */
#define EM_DSP24 136		 /*	New Japan Radio (NJR) 24-bit DSP Processor */
#define EM_VIDEOCORE3 137	 /*	Broadcom VideoCore III processor */
#define EM_LATTICEMICO32 138 /*	RISC processor for Lattice FPGA architecture */
#define EM_SE_C17 139		 /*	Seiko Epson C17 family */
#define EM_TI_C6000 140		 /*	The Texas Instruments TMS320C6000 DSP family */
#define EM_TI_C2000 141		 /*	The Texas Instruments TMS320C2000 DSP family */
#define EM_TI_C5500 142		 /*	The Texas Instruments TMS320C55x DSP family */
#define EM_TI_ARP32 143		 /*	Texas Instruments Application Specific RISC Processor, 32bit fetch */
#define EM_TI_PRU 144		 /*	Texas Instruments Programmable Realtime Unit */
#define EM_MMDSP_PLUS 160	 /*	STMicroelectronics 64bit VLIW Data Signal Processor */
#define EM_CYPRESS_M8C 161	 /*	Cypress M8C microprocessor */
#define EM_R32C 162			 /*	Renesas R32C series microprocessors */
#define EM_TRIMEDIA 163		 /*	NXP Semiconductors TriMedia architecture family */
#define EM_QDSP6 164		 /*	QUALCOMM DSP6 Processor */
#define EM_8051 165			 /*	Intel 8051 and variants */
#define EM_STXP7X 166		 /*	STMicroelectronics STxP7x family of configurable and extensible RISC processors */
#define EM_NDS32 167		 /*	Andes Technology compact code size embedded RISC processor family */
#define EM_ECOG1 168		 /*	Cyan Technology eCOG1X family */
#define EM_ECOG1X 168		 /*	Cyan Technology eCOG1X family */
#define EM_MAXQ30 169		 /*	Dallas Semiconductor MAXQ30 Core Micro-controllers */
#define EM_XIMO16 170		 /*	New Japan Radio (NJR) 16-bit DSP Processor */
#define EM_MANIK 171		 /*	M2000 Reconfigurable RISC Microprocessor */
#define EM_CRAYNV2 172		 /*	Cray Inc. NV2 vector architecture */
#define EM_RX 173			 /*	Renesas RX family */
#define EM_METAG 174		 /*	Imagination Technologies META processor architecture */
#define EM_MCST_ELBRUS 175	 /*	MCST Elbrus general purpose hardware architecture */
#define EM_ECOG16 176		 /*	Cyan Technology eCOG16 family */
#define EM_CR16 177			 /*	National Semiconductor CompactRISC CR16 16-bit microprocessor */
#define EM_ETPU 178			 /*	Freescale Extended Time Processing Unit */
#define EM_SLE9X 179		 /*	Infineon Technologies SLE9X core */
#define EM_L10M 180			 /*	Intel L10M */
#define EM_K10M 181			 /*	Intel K10M */
#define EM_AARCH64 183		 /*	ARM 64-bit architecture (AARCH64) */
#define EM_AVR32 185		 /*	Atmel Corporation 32-bit microprocessor family */
#define EM_STM8 186			 /*	STMicroeletronics STM8 8-bit microcontroller */
#define EM_TILE64 187		 /*	Tilera TILE64 multicore architecture family */
#define EM_TILEPRO 188		 /*	Tilera TILEPro multicore architecture family */
#define EM_MICROBLAZE 189	 /*	Xilinx MicroBlaze 32-bit RISC soft processor core */
#define EM_CUDA 190			 /*	NVIDIA CUDA architecture */
#define EM_TILEGX 191		 /*	Tilera TILE-Gx multicore architecture family */
#define EM_CLOUDSHIELD 192	 /*	CloudShield architecture family */
#define EM_COREA_1ST 193	 /*	KIPO-KAIST Core-A 1st generation processor family */
#define EM_COREA_2ND 194	 /*	KIPO-KAIST Core-A 2nd generation processor family */
#define EM_ARC_COMPACT2 195	 /*	Synopsys ARCompact V2 */
#define EM_OPEN8 196		 /*	Open8 8-bit RISC soft processor core */
#define EM_RL78 197			 /*	Renesas RL78 family */
#define EM_VIDEOCORE5 198	 /*	Broadcom VideoCore V processor */
#define EM_78KOR 199		 /*	Renesas 78KOR family */
#define EM_56800EX 200		 /*	Freescale 56800EX Digital Signal Controller (DSC) */
#define EM_BA1 201			 /*	Beyond BA1 CPU architecture */
#define EM_BA2 202			 /*	Beyond BA2 CPU architecture */
#define EM_XCORE 203		 /*	XMOS xCORE processor family */
#define EM_MCHP_PIC 204		 /*	Microchip 8-bit PIC(r) family */
#define EM_INTEL205 205		 /*	Reserved by Intel */
#define EM_INTEL206 206		 /*	Reserved by Intel */
#define EM_INTEL207 207		 /*	Reserved by Intel */
#define EM_INTEL208 208		 /*	Reserved by Intel */
#define EM_INTEL209 209		 /*	Reserved by Intel */
#define EM_KM32 210			 /*	KM211 KM32 32-bit processor */
#define EM_KMX32 211		 /*	KM211 KMX32 32-bit processor */
#define EM_KMX16 212		 /*	KM211 KMX16 16-bit processor */
#define EM_KMX8 213			 /*	KM211 KMX8 8-bit processor */
#define EM_KVARC 214		 /*	KM211 KVARC processor */
#define EM_CDP 215			 /*	Paneve CDP architecture family */
#define EM_COGE 216			 /*	Cognitive Smart Memory Processor */
#define EM_COOL 217			 /*	Bluechip Systems CoolEngine */
#define EM_NORC 218			 /*	Nanoradio Optimized RISC */
#define EM_CSR_KALIMBA 219	 /*	CSR Kalimba architecture family */
#define EM_Z80 220			 /*	Zilog Z80 */
#define EM_VISIUM 221		 /*	Controls and Data Services VISIUMcore processor */
#define EM_FT32 222			 /*	FTDI Chip FT32 high performance 32-bit RISC architecture */
#define EM_MOXIE 223		 /*	Moxie processor family */
#define EM_AMDGPU 224		 /*	AMD GPU architecture */
#define EM_RISCV 243		 /*	RISC-V */

#define EV_NONE 0x0	   /* Invalid ELF Version */
#define EV_CURRENT 0x1 /* ELF Current Version */

#define ELFMAG0 0x7F
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

#define ELFCLASSNONE 0
#define ELFCLASS32 1
#define ELFCLASS64 2
#define ELFCLASSNUM 3

enum SpecialSections
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

#define NT_PRSTATUS 1
#define NT_PRFPREG 2
#define NT_FPREGSET 2
#define NT_PRPSINFO 3
#define NT_PRXREG 4
#define NT_TASKSTRUCT 4
#define NT_PLATFORM 5
#define NT_AUXV 6
#define NT_GWINDOWS 7
#define NT_ASRS 8
#define NT_PSTATUS 10
#define NT_PSINFO 13
#define NT_PRCRED 14
#define NT_UTSNAME 15
#define NT_LWPSTATUS 16
#define NT_LWPSINFO 17
#define NT_PRFPXREG 20
#define NT_SIGINFO 0x53494749
#define NT_FILE 0x46494c45
#define NT_PRXFPREG 0x46e62b7f
#define NT_PPC_VMX 0x100
#define NT_PPC_SPE 0x101
#define NT_PPC_VSX 0x102
#define NT_PPC_TAR 0x103
#define NT_PPC_PPR 0x104
#define NT_PPC_DSCR 0x105
#define NT_PPC_EBB 0x106
#define NT_PPC_PMU 0x107
#define NT_PPC_TM_CGPR 0x108
#define NT_PPC_TM_CFPR 0x109
#define NT_PPC_TM_CVMX 0x10a
#define NT_PPC_TM_CVSX 0x10b
#define NT_PPC_TM_SPR 0x10c
#define NT_PPC_TM_CTAR 0x10d
#define NT_PPC_TM_CPPR 0x10e
#define NT_PPC_TM_CDSCR 0x10f
#define NT_386_TLS 0x200
#define NT_386_IOPERM 0x201
#define NT_X86_XSTATE 0x202
#define NT_S390_HIGH_GPRS 0x300
#define NT_S390_TIMER 0x301
#define NT_S390_TODCMP 0x302
#define NT_S390_TODPREG 0x303
#define NT_S390_CTRS 0x304
#define NT_S390_PREFIX 0x305
#define NT_S390_LAST_BREAK 0x306
#define NT_S390_SYSTEM_CALL 0x307
#define NT_S390_TDB 0x308
#define NT_S390_VXRS_LOW 0x309
#define NT_S390_VXRS_HIGH 0x30a
#define NT_S390_GS_CB 0x30b
#define NT_S390_GS_BC 0x30c
#define NT_S390_RI_CB 0x30d
#define NT_ARM_VFP 0x400
#define NT_ARM_TLS 0x401
#define NT_ARM_HW_BREAK 0x402
#define NT_ARM_HW_WATCH 0x403
#define NT_ARM_SYSTEM_CALL 0x404
#define NT_ARM_SVE 0x405
#define NT_ARM_PAC_MASK 0x406
#define NT_ARM_PACA_KEYS 0x407
#define NT_ARM_PACG_KEYS 0x408
#define NT_ARM_TAGGED_ADDR_CTRL 0x409
#define NT_ARM_PAC_ENABLED_KEYS 0x40a
#define NT_METAG_CBUF 0x500
#define NT_METAG_RPIPE 0x501
#define NT_METAG_TLS 0x502
#define NT_ARC_V2 0x600
#define NT_VMCOREDD 0x700
#define NT_MIPS_DSP 0x800
#define NT_MIPS_FP_MODE 0x801
#define NT_MIPS_MSA 0x802
#define NT_VERSION 1

#define NT_GNU_PROPERTY_TYPE_0 5

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

typedef struct
{
	Elf32_Word p_type;
	Elf32_Off p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	Elf32_Word p_filesz;
	Elf32_Word p_memsz;
	Elf32_Word p_flags;
	Elf32_Word p_align;
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
	Elf64_Addr r_offset;
	Elf64_Xword r_info;
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
	Elf64_Word st_name;
	unsigned char st_info;
	unsigned char st_other;
	Elf64_Half st_shndx;
	Elf64_Addr st_value;
	Elf64_Xword st_size;
} Elf64_Sym;

typedef struct
{
	Elf32_Sword d_tag;
	union
	{
		Elf32_Word d_val;
		Elf32_Addr d_ptr;
	} d_un;
} Elf32_Dyn;

typedef struct
{
	Elf64_Sxword d_tag;
	union
	{
		Elf64_Xword d_val;
		Elf64_Addr d_ptr;
	} d_un;
} Elf64_Dyn;

typedef struct
{
	Elf32_Addr r_offset;
	Elf32_Word r_info;
	Elf32_Sword r_addend;
} Elf32_Rela;

typedef struct
{
	Elf64_Addr r_offset;
	Elf64_Xword r_info;
	Elf64_Sxword r_addend;
} Elf64_Rela;

struct Elf32_Nhdr
{
	Elf32_Word n_namesz;
	Elf32_Word n_descsz;
	Elf32_Word n_type;
};

struct Elf64_Nhdr
{
	Elf64_Word n_namesz;
	Elf64_Word n_descsz;
	Elf64_Word n_type;
};

typedef struct
{
	Elf32_Sword si_signo;
	Elf32_Sword si_code;
	Elf32_Sword si_errno;
} Elf32_Siginfo;

typedef struct
{
	Elf64_Sword si_signo;
	Elf64_Sword si_code;
	Elf64_Sword si_errno;
} Elf64_Siginfo;

typedef struct
{
	Elf32_Sword tv_sec;
	Elf32_Sword tv_usec;
} Elf32_Prtimeval;

typedef struct
{
	Elf64_Sxword tv_sec;
	Elf64_Sxword tv_usec;
} Elf64_Prtimeval;

typedef struct
{
	Elf32_Siginfo pr_info;
	Elf32_Half pr_cursig;
	Elf32_Word pr_sigpend;
	Elf32_Word pr_sighold;
	Elf32_pid_t pr_pid;
	Elf32_pid_t pr_ppid;
	Elf32_pid_t pr_pgrp;
	Elf32_pid_t pr_sid;
	Elf32_Prtimeval pr_utime;
	Elf32_Prtimeval pr_stime;
	Elf32_Prtimeval pr_cutime;
	Elf32_Prtimeval pr_cstime;
	Elf32_greg_t pr_reg;
	Elf32_Word pr_fpvalid;
} Elf32_Prstatus;

typedef struct
{
	Elf64_Siginfo pr_info;
	Elf64_Half pr_cursig;
	Elf64_Word pr_sigpend;
	Elf64_Word pr_sighold;
	Elf64_pid_t pr_pid;
	Elf64_pid_t pr_ppid;
	Elf64_pid_t pr_pgrp;
	Elf64_pid_t pr_sid;
	Elf64_Prtimeval pr_utime;
	Elf64_Prtimeval pr_stime;
	Elf64_Prtimeval pr_cutime;
	Elf64_Prtimeval pr_cstime;
	Elf64_greg_t pr_reg;
	Elf64_Word pr_fpvalid;
} Elf64_Prstatus;

#define ELF_PRARGSZ 80
typedef struct
{
	char pr_state;
	char pr_sname;
	char pr_zomb;
	char pr_nice;
	Elf32_Word pr_flag;
	Elf32_Half pr_uid;
	Elf32_Half pr_gid;
	Elf32_pid_t pr_pid;
	Elf32_pid_t pr_ppid;
	Elf32_pid_t pr_pgrp;
	Elf32_pid_t pr_sid;
	char pr_fname[16];
	char pr_psargs[ELF_PRARGSZ];
} Elf32_Prpsinfo;

typedef struct
{
	char pr_state;
	char pr_sname;
	char pr_zomb;
	char pr_nice;
	Elf64_Xword pr_flag;
	Elf64_Half pr_uid;
	Elf64_Half pr_gid;
	Elf64_pid_t pr_pid;
	Elf64_pid_t pr_ppid;
	Elf64_pid_t pr_pgrp;
	Elf64_pid_t pr_sid;
	char pr_fname[16];
	char pr_psargs[ELF_PRARGSZ];
} Elf64_Prpsinfo;

#ifdef __LP64__
typedef Elf64_Addr Elf_Addr;
typedef Elf64_Half Elf_Half;
typedef Elf64_Off Elf_Off;
typedef Elf64_Sword Elf_Sword;
typedef Elf64_Word Elf_Word;

typedef Elf64_Ehdr Elf_Ehdr;
typedef Elf64_Shdr Elf_Shdr;
typedef Elf64_Phdr Elf_Phdr;
typedef Elf64_Rel Elf_Rel;
typedef Elf64_Sym Elf_Sym;
typedef Elf64_Dyn Elf_Dyn;
typedef Elf64_Rela Elf_Rela;
typedef Elf64_Nhdr Elf_Nhdr;
typedef Elf64_Prstatus Elf_Prstatus;
typedef Elf64_Prpsinfo Elf_Prpsinfo;
typedef Elf64_Xword Elf_Xword;

#define ELF_ST_BIND(info) ELF64_ST_BIND(info)
#define ELF_ST_TYPE(info) ELF64_ST_TYPE(info)
#define ELF_ST_INFO(bind, type) ELF64_ST_INFO(bind, type)
#define ELF_ST_VISIBILITY(o) ELF64_ST_VISIBILITY(o)
#define ELF_R_SYM(i) ELF64_R_SYM(i)
#define ELF_R_TYPE(i) ELF64_R_TYPE(i)
#define ELF_R_INFO(s, t) ELF64_R_INFO(s, t)
#else
typedef Elf32_Addr Elf_Addr;
typedef Elf32_Half Elf_Half;
typedef Elf32_Off Elf_Off;
typedef Elf32_Sword Elf_Sword;
typedef Elf32_Word Elf_Word;

typedef Elf32_Ehdr Elf_Ehdr;
typedef Elf32_Shdr Elf_Shdr;
typedef Elf32_Phdr Elf_Phdr;
typedef Elf32_Rel Elf_Rel;
typedef Elf32_Sym Elf_Sym;
typedef Elf32_Dyn Elf_Dyn;
typedef Elf32_Rela Elf_Rela;
typedef Elf32_Nhdr Elf_Nhdr;
typedef Elf32_Prstatus Elf_Prstatus;
typedef Elf32_Prpsinfo Elf_Prpsinfo;
typedef Elf32_Word Elf_Xword;

#define ELF_ST_BIND(info) ELF32_ST_BIND(info)
#define ELF_ST_TYPE(info) ELF32_ST_TYPE(info)
#define ELF_ST_INFO(bind, type) ELF32_ST_INFO(bind, type)
#define ELF_ST_VISIBILITY(o) ELF32_ST_VISIBILITY(o)
#define ELF_R_SYM(i) ELF32_R_SYM(i)
#define ELF_R_TYPE(i) ELF32_R_TYPE(i)
#define ELF_R_INFO(s, t) ELF32_R_INFO(s, t)
#endif

#endif // !__FENNIX_KERNEL_ELF_H__
