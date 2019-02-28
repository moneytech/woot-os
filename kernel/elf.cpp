#include <debug.hpp>
#include <elf.hpp>
#include <file.hpp>
#include <memory.hpp>
#include <paging.hpp>
#include <process.hpp>
#include <string.hpp>
#include <stringbuilder.hpp>
#include <sysdefs.h>

static const char *libDir = "WOOT_OS:/lib";

ELF::ELF(const char *name, Elf32_Ehdr *ehdr, uint8_t *phdrData, uint8_t *shdrData, bool user) :
    Name(String::Duplicate(name)), ehdr(ehdr), phdrData(phdrData), shdrData(shdrData), user(user),
    EntryPoint((int (*)())ehdr->e_entry)
{
}

Elf32_Shdr *ELF::getShdr(int i)
{
    return (Elf32_Shdr *)(shdrData + i * ehdr->e_shentsize);
}

ELF *ELF::Load(const char *filename, bool user, bool onlyHeaders, bool applyRelocs)
{
    File *f = File::Open(filename, O_RDONLY);
    if(!f)
    {
        DEBUG("[elf] Couldn't find '%s' file\n", filename);
        return nullptr;
    }
    Elf32_Ehdr *ehdr = new Elf32_Ehdr;
    if(f->Read(ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr))
    {
        DEBUG("[elf] Couldn't load ELF header\n", filename);
        delete ehdr;
        delete f;
        return nullptr;
    }
    if(ehdr->e_ident[0] != 127 || ehdr->e_ident[1] != 'E' || ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F')
    {
        DEBUG("[elf] Invalid ELF header magic\n", filename);
        delete ehdr;
        delete f;
        return nullptr;
    }
    // load program headers
    if(f->Seek(ehdr->e_phoff, SEEK_SET) != ehdr->e_phoff)
    {
        DEBUG("[elf] Couldn't seek to program headers\n", filename);
        delete ehdr;
        delete f;
        return nullptr;
    }
    size_t phSize = ehdr->e_phentsize * ehdr->e_phnum;
    uint8_t *phdrData = new uint8_t[phSize];
    if(f->Read(phdrData, phSize) != phSize)
    {
        DEBUG("[elf] Couldn't load program headers\n", filename);
        delete[] phdrData;
        delete ehdr;
        delete f;
        return nullptr;
    }

    // load section headers
    if(f->Seek(ehdr->e_shoff, SEEK_SET) != ehdr->e_shoff)
    {
        DEBUG("[elf] Couldn't seek to section headers\n", filename);
        delete[] phdrData;
        delete ehdr;
        delete f;
        return nullptr;
    }
    size_t shSize = ehdr->e_shentsize * ehdr->e_shnum;
    uint8_t *shdrData = new uint8_t[shSize];
    if(f->Read(shdrData, shSize) != shSize)
    {
        DEBUG("[elf] Couldn't load section headers\n", filename);
        delete[] shdrData;
        delete[] phdrData;
        delete ehdr;
        delete f;
        return nullptr;
    }

    ELF *elf = new ELF(filename, ehdr, phdrData, shdrData, user);
    Process *proc = Process::GetCurrent();
    proc->AddELF(elf);

    // calculate boundaries
    uintptr_t lowest_vaddr = ~0;
    uintptr_t highest_vaddr = 0;
    for(int i = 0; i < ehdr->e_phnum; ++i)
    {
        Elf32_Phdr *phdr = (Elf32_Phdr *)(phdrData + ehdr->e_phentsize * i);
        if(phdr->p_type != PT_LOAD)
            continue;
        if(phdr->p_vaddr < lowest_vaddr)
            lowest_vaddr = phdr->p_vaddr;
        if((phdr->p_vaddr + phdr->p_memsz) > highest_vaddr)
            highest_vaddr = phdr->p_vaddr + phdr->p_memsz;
    }
    elf->base = lowest_vaddr = PAGE_SIZE * (lowest_vaddr / PAGE_SIZE);
    elf->top = highest_vaddr = align(highest_vaddr, PAGE_SIZE);

    //DEBUG("%s la: %p ha %p\n", elf->Name, lowest_vaddr, highest_vaddr);

    if(!onlyHeaders && proc)
    {
        elf->process = proc;
        elf->releaseData = true;

        // check if image can be mapped where it wants
        bool fits = true;
        if(lowest_vaddr >= (1 << 20))
        {
            for(uintptr_t va = lowest_vaddr; va < highest_vaddr; va += PAGE_SIZE)
            {
                uintptr_t pa = Paging::GetPhysicalAddress(proc->AddressSpace, va);
                if(pa != ~0)
                {
                    fits = false;
                    break;
                }
            }
        } else fits = false;
        if(!fits)
        {   // nope, we have to find some other place
            uintptr_t candidateStart = user ? max(1 << 20, lowest_vaddr) : lowest_vaddr;

            while(candidateStart < (user ? KERNEL_BASE : 0xFFFFE000))
            {
                uintptr_t candidateEnd = candidateStart + highest_vaddr - lowest_vaddr;
                fits = true;
                size_t checkedBytes = 0;
                for(uintptr_t va = candidateStart; va < candidateEnd; va += PAGE_SIZE)
                {
                    checkedBytes += PAGE_SIZE;
                    uintptr_t pa = Paging::GetPhysicalAddress(proc->AddressSpace, va);
                    if(pa != ~0)
                    {
                        fits = false;
                        break;
                    }
                }
                if(!fits)
                {
                    candidateStart += checkedBytes;
                    continue;
                } else break;
            }

            elf->baseDelta = candidateStart - lowest_vaddr;
        }
        elf->base += elf->baseDelta;
        elf->top += elf->baseDelta;

        // load the data
        for(uint i = 0; i < ehdr->e_phnum; ++i)
        {
            Elf32_Phdr *phdr = (Elf32_Phdr *)(phdrData + ehdr->e_phentsize * i);
            if(phdr->p_type != PT_LOAD)
                continue;
            if(f->Seek(phdr->p_offset, SEEK_SET) != phdr->p_offset)
            {
                DEBUG("[elf] Couldn't seek to data of program header %d in file '%s'\n", i, filename);
                delete f;
                delete[] phdrData;
                delete ehdr;
                delete elf;
                return nullptr;
            }

            uintptr_t endva = phdr->p_vaddr + phdr->p_memsz;
            uintptr_t s = phdr->p_vaddr / PAGE_SIZE;
            uintptr_t e = align(endva, PAGE_SIZE) / PAGE_SIZE;
            size_t pageCount = e - s;
            for(uint i = 0; i < pageCount; ++i)
            {
                uintptr_t va = elf->baseDelta + phdr->p_vaddr + i * PAGE_SIZE;
                if(user && va >= KERNEL_BASE)
                {   // user elf can't map any kernel memory
                    DEBUG("[elf] Invalid user address %p in file '%s'\n", va, filename);
                    delete f;
                    delete[] phdrData;
                    delete ehdr;
                    delete elf;
                    return nullptr;
                }
                uintptr_t pa = Paging::GetPhysicalAddress(proc->AddressSpace, va);
                if(!user && pa != ~0)
                {
                    /*printf("[elf] Address conflict at %p in file '%s'\n", va, filename);
                    delete f;
                    return nullptr;*/
                    continue;
                }
                pa = Paging::AllocPage();
                if(pa == ~0)
                {
                    DEBUG("[elf] Couldn't allocate memory for data in file '%s'\n", filename);
                    delete f;
                    delete[] phdrData;
                    delete ehdr;
                    delete elf;
                    return nullptr;
                }
                if(!Paging::MapPage(proc->AddressSpace, va, pa, user, true))
                {
                    DEBUG("[elf] Couldn't map memory for data in file '%s'\n", filename);
                    delete f;
                    delete[] phdrData;
                    delete ehdr;
                    delete elf;
                    return nullptr;
                }
                elf->endPtr = max(elf->endPtr, va + PAGE_SIZE);
            }
            uint8_t *buffer = (uint8_t *)(phdr->p_vaddr + elf->baseDelta);
            Memory::Zero(buffer, phdr->p_memsz);
            if(f->Read(buffer, phdr->p_filesz) != phdr->p_filesz)
            {
                DEBUG("[elf] Couldn't read data of program header %d in file '%s'\n", i, filename);
                delete f;
                delete[] phdrData;
                delete ehdr;
                delete elf;
                return nullptr;
            }
        }
        delete f;

        // load needed shared objects
        if(user)
        { // ignore DT_NEEDED for kernel modules for now
            // find soname if possible
            for(uint i = 0; i < ehdr->e_shnum; ++i)
            {
                Elf32_Shdr *shdr = elf->getShdr(i);
                if(shdr->sh_type != SHT_DYNAMIC)
                    continue;
                uint8_t *dyntab = (uint8_t *)(shdr->sh_addr + elf->baseDelta);
                char *_strtab = (char *)(elf->getShdr(shdr->sh_link)->sh_addr + elf->baseDelta);
                for(uint coffs = 0; coffs < shdr->sh_size; coffs += shdr->sh_entsize)
                {
                    Elf32_Dyn *dyn = (Elf32_Dyn *)(dyntab + coffs);
                    if(dyn->d_tag != DT_SONAME)
                        continue;
                    if(elf->Name) delete[] elf->Name;
                    char *soname = _strtab + dyn->d_un.d_val;
                    elf->Name = String::Duplicate(soname);
                }
            }

            for(uint i = 0; i < ehdr->e_shnum; ++i)
            {
                Elf32_Shdr *shdr = elf->getShdr(i);
                if(shdr->sh_type != SHT_DYNAMIC)
                    continue;
                uint8_t *dyntab = (uint8_t *)(shdr->sh_addr + elf->baseDelta);
                char *_strtab = (char *)(elf->getShdr(shdr->sh_link)->sh_addr + elf->baseDelta);
                for(uint coffs = 0; coffs < shdr->sh_size; coffs += shdr->sh_entsize)
                {
                    Elf32_Dyn *dyn = (Elf32_Dyn *)(dyntab + coffs);
                    if(dyn->d_tag != DT_NEEDED)
                        continue;
                    char *soname = _strtab + dyn->d_un.d_val;
                    StringBuilder sb(MAX_PATH_LENGTH);
                    sb.WriteFmt("%s/%s", libDir, soname);
                    soname = sb.String();
                    if(proc->GetELF(soname))
                        continue;
                    //DEBUG("[elf] loading DT_NEEDED %s for %s\n", soname, elf->Name);
                    ELF *soELF = Load(soname, user, false, applyRelocs);
                }
            }
        }
    } else delete f;

    delete[] phdrData;
    delete ehdr;

    elf->ehdr = (Elf32_Ehdr *)elf->base;
    elf->phdrData = (uint8_t *)(elf->base + elf->ehdr->e_phoff);

    if(applyRelocs && !elf->ApplyRelocations())
    {
        delete elf;
        return nullptr;
    }

    // adjust entry point
    if(elf->baseDelta)
        elf->EntryPoint = (int (*)())((uint8_t *)elf->EntryPoint + elf->baseDelta);

    return elf;
}

Elf32_Sym *ELF::FindSymbol(const char *name)
{
    for(uint i = 0; i < ehdr->e_shnum; ++i)
    {
        Elf32_Shdr *shdr = getShdr(i);
        if(shdr->sh_type != SHT_DYNSYM)
            continue;
        if(!shdr->sh_addr)
            continue;
        char *strtab = (char *)(getShdr(shdr->sh_link)->sh_addr + baseDelta);
        uint8_t *symtab = (uint8_t *)(shdr->sh_addr + baseDelta);
        for(uint coffs = 0; coffs < shdr->sh_size; coffs += shdr->sh_entsize)
        {
            Elf32_Sym *sym = (Elf32_Sym *)(symtab + coffs);
            //int type = ELF32_ST_TYPE(sym->st_info);
            if(!sym->st_shndx || !sym->st_name)
                continue;
            if(!String::Compare(name, strtab + sym->st_name))
                return sym;
        }
    }
    return nullptr;
}

bool ELF::ApplyRelocations()
{
    for(uint i = 0; i < ehdr->e_shnum; ++i)
    {
        Elf32_Shdr *shdr = getShdr(i);
        if(shdr->sh_type != SHT_REL) // no SHT_RELA on x86
            continue;

        uint8_t *reltab = (uint8_t *)(shdr->sh_addr + baseDelta);
        Elf32_Sym *_symtab = (Elf32_Sym *)(getShdr(shdr->sh_link)->sh_addr + baseDelta);
        char *_strtab = (char *)(getShdr(getShdr(shdr->sh_link)->sh_link)->sh_addr + baseDelta);

        for(uint coffs = 0; coffs < shdr->sh_size; coffs += shdr->sh_entsize)
        {
            Elf32_Rel *rel = (Elf32_Rel *)(reltab + coffs);
            uint symIdx = ELF32_R_SYM(rel->r_info);
            uint rType = ELF32_R_TYPE(rel->r_info);
            Elf32_Sym *symbol = _symtab + symIdx;
            Elf32_Sym *fSymbol = nullptr;

            uintptr_t symAddr = 0;
            char *name = _strtab + symbol->st_name;
            ELF *fElf = nullptr;
            if(symIdx && name[0])
            {
                fSymbol = process->FindSymbol(name, this, &fElf);
                if(!fSymbol)
                {
                    fSymbol = FindSymbol(name);
                    if(fSymbol) fElf = this;
                }
                if(fSymbol)
                    symAddr = fSymbol->st_value + (fElf ? fElf->baseDelta : 0);
                else
                {
                    DEBUG("[elf] Couldn't find symbol '%s' for '%s'\n", name, Name);
                    return false;
                }
            }
            else symAddr = symbol->st_value;

            uintptr_t *val = (uintptr_t *)(rel->r_offset + baseDelta);

            //printf("%s: rel: %d ", Name, rType);
            //printf("sym: %s S: %.8x A: %.8x P: %.8x\n", symbol->st_name ? name : "<no symbol>", S, A, P);
            uintptr_t prevVal = *val;

            switch(rType)
            {
            case R_386_NONE:
                break;
            case R_386_32:
                *val += symAddr;
                break;
            case R_386_PC32:
                *val += symAddr - (uintptr_t)val;
                break;
            case R_386_COPY:
                Memory::Move(val, (void *)symAddr, symbol->st_size);
                break;
            case R_386_GLOB_DAT:
            case R_386_JMP_SLOT:
                *val = symAddr;
                break;
            case R_386_RELATIVE:
                *val += baseDelta;
                break;
            default:
                DEBUG("[elf] Unsupported relocation type: %d in '%s'\n", rType, Name);
                return false;
            }
            //printf("%-16s  %p ->(%d)-> %p\n", name, prevVal, rType, *val);
        }
    }
    return true;
}

uintptr_t ELF::GetBase()
{
    return base;
}

uintptr_t ELF::GetEndPtr()
{
    return endPtr;
}

ELF::~ELF()
{
    if(Name) delete[] Name;
    if(process)
    {
        AddressSpace as = process->AddressSpace;
        for(uintptr_t va = base; va < top; va += PAGE_SIZE)
        {
            uintptr_t pa = Paging::GetPhysicalAddress(as, va);
            Paging::UnMapPage(as, va);
            if(pa == ~0) continue;
            Paging::FreePage(pa);
        }
    }
    if(shdrData) delete[] shdrData;
}
