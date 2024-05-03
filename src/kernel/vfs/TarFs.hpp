//
// Created by Stepan Usatiuk on 01.05.2024.
//

#ifndef TARFS_HPP
#define TARFS_HPP

#include <Filesystem.hpp>

class TarFs : public Filesystem {
public:
     TarFs(char *backing, size_t backing_size, NodeDir *mounted_on);
    ~TarFs() override = default;

    SharedPtr<NodeDir> root() override;
    SharedPtr<Node>    get_node(ino_t inode) override;


    struct TarFsNodeDir : public ::NodeDir {
    public:
        ~TarFsNodeDir() override = default;

        Vector<DirEntry> children() override;
        ino_t            mkdir(const String &name) override;
        ino_t            mkfile(const String &name) override;

        static SharedPtr<TarFsNodeDir> create(Filesystem *fs, ino_t id) {
            auto shared        = SharedPtr(new TarFsNodeDir(fs, id));
            shared->_self_weak = static_ptr_cast<Node>(shared);
            return shared;
        }

    private:
        TarFsNodeDir(Filesystem *fs, ino_t id) : ::NodeDir(fs, id) {}
    };

    struct TarFsNodeFile : public ::NodeFile {
    public:
        ~TarFsNodeFile() override = default;

        int64_t read(char *buf, size_t start, size_t num) override;
        int64_t write(const char *buf, size_t start, size_t num) override;
        size_t  size() override;
        bool    is_tty() override { return false; }

        static SharedPtr<TarFsNodeFile> create(Filesystem *fs, ino_t id) {
            auto shared        = SharedPtr(new TarFsNodeFile(fs, id));
            shared->_self_weak = static_ptr_cast<Node>(shared);
            return shared;
        }

    private:
        TarFsNodeFile(Filesystem *fs, ino_t id) : ::NodeFile(fs, id) {}
    };

private:
    struct tar_header {
        char filename[100];
        char mode[8];
        char uid[8];
        char gid[8];
        char size[12];
        char mtime[12];
        char chksum[8];
        char typeflag[1];
    } __attribute__((packed));

    const tar_header *_backing;
    size_t            _backing_size;

    SkipListMap<ino_t, const tar_header *> _off_map;
    SkipListMap<ino_t, Vector<DirEntry>>   _dir_map;
    // SkipListMap<ino_t, SharedPtr<Node>>    _vnode_map;

    static size_t header_size(const tar_header *hdr);
};


#endif //TARFS_HPP
