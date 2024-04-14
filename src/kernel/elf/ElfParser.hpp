//
// Created by Stepan Usatiuk on 25.03.2024.
//

#ifndef FICUS_ELFPARSER_HPP
#define FICUS_ELFPARSER_HPP

#include "Vector.hpp"

#include <type_traits>

class Task;

// Just copying everytihng for now
class ElfParser {
public:
    ElfParser(Vector<char> data);
    bool copy_to(Task *task);

    uintptr_t get_entrypoint() { return _entrypoint; }

private:
    bool      _valid = false;
    uint8_t   _elf_hdr_ver;
    uint8_t   _abi;
    uint16_t  _elf_ver;
    uintptr_t _entrypoint;
    uintptr_t _phdrs_pos;
    uintptr_t _sects_pos;
    uint32_t  _flags;
    uint16_t  _hdr_size;
    uint16_t  _hdrs_entry_size;
    uint16_t  _hdrs_num;
    uint16_t  _sects_entry_size;
    uint16_t  _sects_num;
    uint16_t  _sects_name_idx;

    using Elf64_Addr   = uintptr_t;
    using Elf64_Off    = size_t;
    using Elf64_Half   = uint16_t;
    using Elf64_Word   = uint32_t;
    using Elf64_Sword  = int32_t;
    using Elf64_Xword  = uint64_t;
    using Elf64_Sxword = int64_t;

    struct Elf64_Phdr {
        Elf64_Word  p_type;   /* Type of segment */
        Elf64_Word  p_flags;  /* Segment attributes */
        Elf64_Off   p_offset; /* Offset in file */
        Elf64_Addr  p_vaddr;  /* Virtual address in memory */
        Elf64_Addr  p_paddr;  /* Reserved */
        Elf64_Xword p_filesz; /* Size of segment in file */
        Elf64_Xword p_memsz;  /* Size of segment in memory */
        Elf64_Xword p_align;  /* Alignment of segment */

        bool valid         = false;
        using serializable = std::true_type;
        Elf64_Phdr(Vector<char>::const_iterator &in, const Vector<char>::const_iterator &end);
    };

    Vector<Elf64_Phdr> _headers;
    Vector<char>       _data;
};


#endif //FICUS_ELFPARSER_HPP
