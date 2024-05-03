//
// Created by Stepan Usatiuk on 01.05.2024.
//

#include "TarFs.hpp"

#include <TtyManager.hpp>


TarFs::TarFs(char *backing, size_t backing_size, NodeDir *mounted_on) : Filesystem(mounted_on), _backing((tar_header *) backing), _backing_size(backing_size) {
    int i = 2;

    const tar_header *header = _backing;

    SkipListMap<String, ino_t> dir_map;

    dir_map.emplace("", 1);
    _dir_map.emplace(1, Vector<DirEntry>());

    // FIXME: all this is extremely inefficient
    // Also recheck that directories are handled correctly
    while (true) {
        if (header->filename[0] == '\0')
            break;
        unsigned int size = header_size(header);

        int new_ino = i++;
        _off_map.emplace(new_ino, header);

        String pref = "";
        String name = "";

        {
            Path p = StrToPath(header->filename);
            if (p.size() > 1) {
                pref += p[0];
                for (int i = 1; i < p.size() - 1; i++) {
                    pref += "/";
                    pref += p[i];
                }
            }
            name = p.back();
        }

        if (header->typeflag[0] == '5') {
            String fullname;
            if (pref.length() > 0) {
                fullname += pref;
                fullname += "/";
            }
            fullname += name;
            if (dir_map.find(fullname) == dir_map.end()) {
                dir_map.emplace(fullname, new_ino);
                _dir_map.emplace(new_ino, Vector<DirEntry>());
            }
            if (dir_map.find(pref) == dir_map.end()) {
                int dir_ino = i++;
                dir_map.emplace(pref, dir_ino);
                _dir_map.emplace(dir_ino, Vector<DirEntry>());
            }
            auto p = dir_map.find(pref);
            auto d = _dir_map.find(p->second);
            d->second.emplace_back(name, new_ino);
        } else if (header->typeflag[0] == '\0' || header->typeflag[0] == '0') {
            if (dir_map.find(pref) == dir_map.end()) {
                int dir_ino = i++;
                dir_map.emplace(pref, dir_ino);
                _dir_map.emplace(dir_ino, Vector<DirEntry>());
            }
            auto p = dir_map.find(pref);
            auto d = _dir_map.find(p->second);
            d->second.emplace_back(name, new_ino);
        } else {
            GlobalTtyManager.all_tty_putstr("Skipping tar: ");
            GlobalTtyManager.all_tty_putstr(pref.c_str());
            GlobalTtyManager.all_tty_putstr("/");
            GlobalTtyManager.all_tty_putstr(name.c_str());
            GlobalTtyManager.all_tty_putstr("\n");
        }

        header = (const tar_header *) (((char *) (header)) + ((size / 512) + 1) * 512);

        if (size % 512)
            header = (const tar_header *) (((char *) (header)) + 512);
    }
}

SharedPtr<NodeDir> TarFs::root() {
    return static_ptr_cast<NodeDir>(get_node(1));
}

SharedPtr<Node> TarFs::get_node(ino_t inode) {
    if (_dir_map.find(inode) != _dir_map.end()) {
        return static_ptr_cast<Node>(TarFsNodeDir::create(this, inode));
    } else {
        return static_ptr_cast<Node>(TarFsNodeFile::create(this, inode));
    }
    assert(false);
}

Vector<DirEntry> TarFs::TarFsNodeDir::children() {
    return static_cast<TarFs *>(_fs)->_dir_map.find(_ino)->second;
}

ino_t TarFs::TarFsNodeDir::mkdir(const String &name) {
    return -1;
}
ino_t TarFs::TarFsNodeDir::mkfile(const String &name) {
    return -1;
}

int64_t TarFs::TarFsNodeFile::read(char *buf, size_t start, size_t num) {
    TarFs *tar_fs = static_cast<TarFs *>(_fs);

    if (auto entry = tar_fs->_off_map.find(_ino); entry != tar_fs->_off_map.end()) {
        num = std::min(num, size() - start);
        for (size_t i = 0; i < num; i++) {
            buf[i] = (((char *) entry->second) + 512)[start + i];
        }
        return num;
    }
    assert(false);
}

int64_t TarFs::TarFsNodeFile::write(const char *buf, size_t start, size_t num) {
    return -1;
}

size_t TarFs::TarFsNodeFile::size() {
    TarFs *tar_fs = static_cast<TarFs *>(_fs);

    if (auto entry = tar_fs->_off_map.find(_ino); entry != tar_fs->_off_map.end()) {
        return header_size(entry->second);
    }

    assert(false);
}

size_t TarFs::header_size(const tar_header *hdr) {
    int size = 0;

    size_t count = 1;
    for (int j = 11; j > 0; j--, count *= 8)
        size += ((hdr->size[j - 1] - '0') * count);

    return size;
}
