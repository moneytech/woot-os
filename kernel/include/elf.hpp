#pragma once

#include <types.hpp>

#pragma pack(push, 1)

typedef uint16_t Elf32_Half;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf32_Word;
typedef	int32_t  Elf32_Sword;
typedef uint32_t Elf64_Word;
typedef	int32_t  Elf64_Sword;
typedef uint64_t Elf32_Xword;
typedef	int64_t  Elf32_Sxword;
typedef uint64_t Elf64_Xword;
typedef	int64_t  Elf64_Sxword;
typedef uint32_t Elf32_Addr;
typedef uint64_t Elf64_Addr;
typedef uint32_t Elf32_Off;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf32_Section;
typedef uint16_t Elf64_Section;
typedef Elf32_Half Elf32_Versym;
typedef Elf64_Half Elf64_Versym;

/* The ELF file header.  This appears at the start of every ELF file.  */

#define EI_NIDENT (16)

typedef struct
{
    unsigned char   e_ident[EI_NIDENT];	/* Magic number and other info */
    Elf32_Half      e_type;             /* Object file type */
    Elf32_Half      e_machine;          /* Architecture */
    Elf32_Word      e_version;          /* Object file version */
    Elf32_Addr      e_entry;            /* Entry point virtual address */
    Elf32_Off       e_phoff;            /* Program header table file offset */
    Elf32_Off       e_shoff;            /* Section header table file offset */
    Elf32_Word      e_flags;            /* Processor-specific flags */
    Elf32_Half      e_ehsize;           /* ELF header size in bytes */
    Elf32_Half      e_phentsize;        /* Program header table entry size */
    Elf32_Half      e_phnum;            /* Program header table entry count */
    Elf32_Half      e_shentsize;        /* Section header table entry size */
    Elf32_Half      e_shnum;            /* Section header table entry count */
    Elf32_Half      e_shstrndx;         /* Section header string table index */
} Elf32_Ehdr;

typedef struct
{
    Elf32_Word	p_type;
    Elf32_Off	p_offset;
    Elf32_Addr	p_vaddr;
    Elf32_Addr	p_paddr;
    Elf32_Word	p_filesz;
    Elf32_Word	p_memsz;
    Elf32_Word	p_flags;
    Elf32_Word	p_align;
} Elf32_Phdr;

typedef struct
{
    Elf32_Word	sh_name;		/* Section name (string tbl index) */
    Elf32_Word	sh_type;		/* Section type */
    Elf32_Word	sh_flags;		/* Section flags */
    Elf32_Addr	sh_addr;		/* Section virtual addr at execution */
    Elf32_Off	sh_offset;		/* Section file offset */
    Elf32_Word	sh_size;		/* Section size in bytes */
    Elf32_Word	sh_link;		/* Link to another section */
    Elf32_Word	sh_info;		/* Additional section information */
    Elf32_Word	sh_addralign;	/* Section alignment */
    Elf32_Word	sh_entsize;		/* Entry size if section holds table */
} Elf32_Shdr;

typedef struct
{
    Elf32_Sword	d_tag;
    union
    {
        Elf32_Word	d_val;
        Elf32_Addr	d_ptr;
    } d_un;
} Elf32_Dyn;

typedef struct
{
    Elf32_Word      st_name;		/* Symbol name (string tbl index) */
    Elf32_Addr      st_value;		/* Symbol value */
    Elf32_Word      st_size;		/* Symbol size */
    unsigned char	st_info;		/* Symbol type and binding */
    unsigned char	st_other;		/* Symbol visibility */
    Elf32_Section	st_shndx;		/* Section index */
} Elf32_Sym;

typedef struct
{
    Elf32_Addr      r_offset;
    Elf32_Word      r_info;
} Elf32_Rel;

typedef struct
{
    Elf32_Addr      r_offset;
    Elf32_Word      r_info;
    Elf32_Sword     r_addend;
} Elf32_Rela;

#define PT_NULL     0
#define PT_LOAD     1
#define PT_DYNAMIC	2
#define PT_INTERP	3
#define PT_NOTE     4
#define PT_SHLIB	5
#define PT_PHDR     6
#define PT_TLS      7
#define PT_LOOS     0x60000000
#define PT_HIOS     0x6fffffff
#define PT_LOPROC	0x70000000
#define PT_HIPROC	0x7fffffff

#define SHT_NULL            0       /* Section header table entry unused */
#define SHT_PROGBITS        1		/* Program data */
#define SHT_SYMTAB          2		/* Symbol table */
#define SHT_STRTAB          3		/* String table */
#define SHT_RELA            4		/* Relocation entries with addends */
#define SHT_HASH            5		/* Symbol hash table */
#define SHT_DYNAMIC         6		/* Dynamic linking information */
#define SHT_NOTE            7		/* Notes */
#define SHT_NOBITS          8		/* Program space with no data (bss) */
#define SHT_REL             9		/* Relocation entries, no addends */
#define SHT_SHLIB           10		/* Reserved */
#define SHT_DYNSYM          11		/* Dynamic linker symbol table */
#define SHT_INIT_ARRAY      14		/* Array of constructors */
#define SHT_FINI_ARRAY      15		/* Array of destructors */
#define SHT_PREINIT_ARRAY   16		/* Array of pre-constructors */
#define SHT_GROUP           17		/* Section group */
#define SHT_SYMTAB_SHNDX    18		/* Extended section indeces */
#define	SHT_NUM             19		/* Number of defined types.  */

/* Legal values for d_tag (dynamic entry type).  */

#define DT_NULL             0		/* Marks end of dynamic section */
#define DT_NEEDED           1		/* Name of needed library */
#define DT_PLTRELSZ         2		/* Size in bytes of PLT relocs */
#define DT_PLTGOT           3		/* Processor defined value */
#define DT_HASH             4		/* Address of symbol hash table */
#define DT_STRTAB           5		/* Address of string table */
#define DT_SYMTAB           6		/* Address of symbol table */
#define DT_RELA             7		/* Address of Rela relocs */
#define DT_RELASZ           8		/* Total size of Rela relocs */
#define DT_RELAENT          9		/* Size of one Rela reloc */
#define DT_STRSZ            10		/* Size of string table */
#define DT_SYMENT           11		/* Size of one symbol table entry */
#define DT_INIT             12		/* Address of init function */
#define DT_FINI             13		/* Address of termination function */
#define DT_SONAME           14		/* Name of shared object */
#define DT_RPATH            15		/* Library search path (deprecated) */
#define DT_SYMBOLIC         16		/* Start symbol search here */
#define DT_REL              17		/* Address of Rel relocs */
#define DT_RELSZ            18		/* Total size of Rel relocs */
#define DT_RELENT           19		/* Size of one Rel reloc */
#define DT_PLTREL           20		/* Type of reloc in PLT */
#define DT_DEBUG            21		/* For debugging; unspecified */
#define DT_TEXTREL          22		/* Reloc might modify .text */
#define DT_JMPREL           23		/* Address of PLT relocs */
#define	DT_BIND_NOW         24		/* Process relocations of object */
#define	DT_INIT_ARRAY       25		/* Array with addresses of init fct */
#define	DT_FINI_ARRAY       26		/* Array with addresses of fini fct */
#define	DT_INIT_ARRAYSZ     27		/* Size in bytes of DT_INIT_ARRAY */
#define	DT_FINI_ARRAYSZ     28		/* Size in bytes of DT_FINI_ARRAY */
#define DT_RUNPATH          29		/* Library search path */
#define DT_FLAGS            30		/* Flags for the object being loaded */
#define DT_ENCODING         32		/* Start of encoded range */
#define DT_PREINIT_ARRAY    32		/* Array with addresses of preinit fct*/
#define DT_PREINIT_ARRAYSZ  33		/* size in bytes of DT_PREINIT_ARRAY */
#define	DT_NUM              34		/* Number used */

#define ELF32_ST_BIND(i) ((i) >> 4)
#define ELF32_ST_TYPE(i) ((i) & 0xf)
#define ELF32_ST_INFO(b, t) (((b) << 4) + ((t) & 0xf)

#define STT_NOTYPE  0
#define STT_OBJECT  1
#define STT_FUNC    2
#define STT_SECTION 3
#define STT_FILE    4
#define STT_LOPROC  13
#define STT_HIPROC  15

#define STB_LOCAL   0
#define STB_GLOBAL  1
#define STB_WEAK    2
#define STB_LOPROC  13
#define STB_HIPROC  15

#define ELF32_R_SYM(info)             ((info)>>8)
#define ELF32_R_TYPE(info)            ((unsigned char)(info))
#define ELF32_R_INFO(sym, type)       (((sym)<<8)+(unsigned char)(type))

#define R_386_NONE          0		/* No reloc */
#define R_386_32            1		/* Direct 32 bit  */
#define R_386_PC32          2		/* PC relative 32 bit */
#define R_386_GOT32         3		/* 32 bit GOT entry */
#define R_386_PLT32         4		/* 32 bit PLT address */
#define R_386_COPY          5		/* Copy symbol at runtime */
#define R_386_GLOB_DAT      6		/* Create GOT entry */
#define R_386_JMP_SLOT      7		/* Create PLT entry */
#define R_386_RELATIVE      8		/* Adjust by program base */
#define R_386_GOTOFF        9		/* 32 bit offset to GOT */
#define R_386_GOTPC         10		/* 32 bit PC relative offset to GOT */
#define R_386_32PLT         11
#define R_386_TLS_TPOFF     14		/* Offset in static TLS block */
#define R_386_TLS_IE        15		/* Address of GOT entry for static TLS block offset */
#define R_386_TLS_GOTIE     16		/* GOT entry for static TLS block offset */
#define R_386_TLS_LE        17		/* Offset relative to static TLS block */
#define R_386_TLS_GD        18		/* Direct 32 bit for GNU version of general dynamic thread local data */
#define R_386_TLS_LDM       19		/* Direct 32 bit for GNU version of local dynamic thread local data in LE code */
#define R_386_16            20
#define R_386_PC16          21
#define R_386_8             22
#define R_386_PC8           23
#define R_386_TLS_GD_32     24		/* Direct 32 bit for general dynamic thread local data */
#define R_386_TLS_GD_PUSH   25		/* Tag for pushl in GD TLS code */
#define R_386_TLS_GD_CALL   26		/* Relocation for call to __tls_get_addr() */
#define R_386_TLS_GD_POP    27		/* Tag for popl in GD TLS code */
#define R_386_TLS_LDM_32    28		/* Direct 32 bit for local dynamic thread local data in LE code */
#define R_386_TLS_LDM_PUSH  29		/* Tag for pushl in LDM TLS code */
#define R_386_TLS_LDM_CALL  30		/* Relocation for call to __tls_get_addr() in LDM code */
#define R_386_TLS_LDM_POP   31		/* Tag for popl in LDM TLS code */
#define R_386_TLS_LDO_32    32		/* Offset relative to TLS block */
#define R_386_TLS_IE_32     33		/* GOT entry for negated static TLS block offset */
#define R_386_TLS_LE_32     34		/* Negated offset relative to static TLS block */
#define R_386_TLS_DTPMOD32  35		/* ID of module containing symbol */
#define R_386_TLS_DTPOFF32  36		/* Offset in TLS block */
#define R_386_TLS_TPOFF32   37		/* Negated offset in static TLS block */
#define R_386_SIZE32        38 		/* 32-bit symbol size */
#define R_386_TLS_GOTDESC   39		/* GOT offset for TLS descriptor. */
#define R_386_TLS_DESC_CALL 40		/* Marker of call through TLS descriptor for relaxation. */
#define R_386_TLS_DESC      41		/* TLS descriptor containing pointer to code and to argument, returning the TLS offset for the symbol. */
#define R_386_IRELATIVE     42		/* Adjust indirectly by program base */
#define R_386_GOT32X        43		/* Load from 32 bit GOT entry, relaxable. */
#define R_386_NUM           44      /* Keep this the last entry. */

#define BFD_HAS_RELOC      0x01
#define BFD_EXEC_P         0x02
#define BFD_HAS_SYMS       0x10
#define BFD_D_PAGED        0x100

#pragma pack(pop)

class DEntry;
class Process;

class ELF
{
    Elf32_Ehdr *ehdr;
    uint8_t *phdrData;
    uint8_t *shdrData;
    uintptr_t base;
    uintptr_t baseDelta;
    bool releaseData;
    Process *process;
    bool user;
    uintptr_t endPtr;
    ELF(const char *name, Elf32_Ehdr *ehdr, uint8_t *phdrData, uint8_t *shdrData, bool user);
    Elf32_Shdr *getShdr(int i);
public:
    char *Name;
    int (*EntryPoint)();
    void (*CleanupProc)();
    static ELF *Load(DEntry *dentry, const char *filename, bool user, bool onlyHeaders);

    Elf32_Sym *FindSymbol(const char *Name);
    bool ApplyRelocations();
    uintptr_t GetBase();
    uintptr_t GetEndPtr();
    ~ELF();
};
