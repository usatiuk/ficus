//
// Created by Stepan Usatiuk on 25.03.2024.
//

#include "ElfParser.hpp"

#include "Serialize.hpp"

#include "VMA.hpp"
#include "paging.hpp"
#include "task.hpp"

ElfParser::ElfParser(Vector<char> data) {
    auto it = data.cbegin();
    if (Serialize::deserialize<char>(it, data.end()) != 0x7F) return;
    if (Serialize::deserialize<char>(it, data.end()) != 'E') return;
    if (Serialize::deserialize<char>(it, data.end()) != 'L') return;
    if (Serialize::deserialize<char>(it, data.end()) != 'F') return;
    if (Serialize::deserialize<char>(it, data.end()) != 2) return;
    if (Serialize::deserialize<char>(it, data.end()) != 1) return;
    if (auto val = Serialize::deserialize<uint8_t>(it, data.end()))
        _elf_hdr_ver = *val;
    else
        return;
    if (auto val = Serialize::deserialize<uint8_t>(it, data.end()))
        _abi = *val;
    else
        return;
    if (std::distance(it, data.cend()) < 8) return;
    std::advance(it, 8);

    if (Serialize::deserialize<uint16_t>(it, data.end()) != 2) return;
    if (Serialize::deserialize<uint16_t>(it, data.end()) != 0x3E) return;
    if (auto val = Serialize::deserialize<uint32_t>(it, data.end()))
        _elf_hdr_ver = *val;
    else
        return;

    if (auto val = Serialize::deserialize<uintptr_t>(it, data.end()))
        _entrypoint = *val;
    else
        return;
    if (auto val = Serialize::deserialize<uintptr_t>(it, data.end()))
        _phdrs_pos = *val;
    else
        return;
    if (auto val = Serialize::deserialize<uintptr_t>(it, data.end()))
        _sects_pos = *val;
    else
        return;

    if (auto val = Serialize::deserialize<uint32_t>(it, data.end()))
        _flags = *val;
    else
        return;

    if (auto val = Serialize::deserialize<uint16_t>(it, data.end()))
        _hdr_size = *val;
    else
        return;
    if (auto val = Serialize::deserialize<uint16_t>(it, data.end()))
        _hdrs_entry_size = *val;
    else
        return;
    if (auto val = Serialize::deserialize<uint16_t>(it, data.end()))
        _hdrs_num = *val;
    else
        return;
    if (auto val = Serialize::deserialize<uint16_t>(it, data.end()))
        _sects_entry_size = *val;
    else
        return;
    if (auto val = Serialize::deserialize<uint16_t>(it, data.end()))
        _sects_num = *val;
    else
        return;
    if (auto val = Serialize::deserialize<uint16_t>(it, data.end()))
        _sects_name_idx = *val;
    else
        return;

    auto hdr_begin = data.cbegin();
    std::advance(hdr_begin, _phdrs_pos);

    for (int i = 0; i < _hdrs_num; i++) {
        auto hdr = Serialize::deserialize<Elf64_Phdr>(hdr_begin, data.end());
        if (!hdr->valid) return;
        _headers.emplace_back(*hdr);
    }

    _data  = std::move(data);
    _valid = true;
}

bool ElfParser::copy_to(Task *task) {
    if (!_valid) return false;
    for (const auto &hdr: _headers) {
        if (hdr.p_type == 1 /*PT_LOAD*/) {
            // For some reason, if a segment is empty it disregards it's supposed position in the linker script
            // and everything breaks
            if (hdr.p_memsz == 0) continue;
            uint32_t flags = 0;
            if (hdr.p_flags & 0x1 /*eXecute*/) {
                // TODO:
            }
            if (hdr.p_flags & 0x2 /*Write*/) {
                flags |= PAGE_RW;
            }

            auto rounded_vaddr = hdr.p_vaddr & 0x000FFFFFFFFFF000ULL;
            auto real_memsz    = PAGE_ROUND_UP(hdr.p_memsz + (hdr.p_vaddr - rounded_vaddr));

            uintptr_t real_ptr = reinterpret_cast<uintptr_t>(task->_vma->mmap_mem(reinterpret_cast<void *>(rounded_vaddr), real_memsz, 0, flags | PAGE_USER));
            if (real_ptr != rounded_vaddr) return false;

            auto *file_ptr = _data.begin();
            std::advance(file_ptr, hdr.p_offset);

            char *task_mem = reinterpret_cast<char *>(real_ptr + (hdr.p_vaddr - rounded_vaddr));

            for (size_t i = 0; i < hdr.p_memsz; i++, task_mem++) {
                char *real_ptr = (char *) HHDM_P2V(task->_addressSpace->virt2real(
                                                           reinterpret_cast<void *>((uintptr_t) task_mem & 0x000FFFFFFFFFF000ULL)) +
                                                   ((uintptr_t) task_mem & 0xFFF));
                if (i >= hdr.p_filesz) {
                    *real_ptr = 0;
                } else {
                    *real_ptr = *(file_ptr + i);
                }
            }
        }
    }
    return true;
}

ElfParser::Elf64_Phdr::Elf64_Phdr(Vector<char>::const_iterator &in, Vector<char>::const_iterator const &end) {
    if (auto val = Serialize::deserialize<Elf64_Word>(in, end))
        p_type = *val;
    else
        return;
    if (auto val = Serialize::deserialize<Elf64_Word>(in, end))
        p_flags = *val;
    else
        return;
    if (auto val = Serialize::deserialize<Elf64_Off>(in, end))
        p_offset = *val;
    else
        return;
    if (auto val = Serialize::deserialize<Elf64_Addr>(in, end))
        p_vaddr = *val;
    else
        return;
    if (auto val = Serialize::deserialize<Elf64_Addr>(in, end))
        p_paddr = *val;
    else
        return;
    if (auto val = Serialize::deserialize<Elf64_Xword>(in, end))
        p_filesz = *val;
    else
        return;
    if (auto val = Serialize::deserialize<Elf64_Xword>(in, end))
        p_memsz = *val;
    else
        return;
    if (auto val = Serialize::deserialize<Elf64_Xword>(in, end))
        p_align = *val;
    else
        return;

    valid = true;
}
