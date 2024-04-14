//
// Created by Stepan Usatiuk on 05.04.2024.
//

#ifndef FICUS_TTYPIPE_HPP
#define FICUS_TTYPIPE_HPP


#include "Node.hpp"
class TtyPipe : public NodeFile {
public:
    int64_t read(char *buf, size_t start, size_t num) override;
    int64_t write(const char *buf, size_t start, size_t num) override;
    size_t  size() override;
    bool    is_tty() override { return true; }

    static SharedPtr<TtyPipe> create() {
        auto shared        = SharedPtr(new TtyPipe());
        shared->_self_weak = static_ptr_cast<Node>(shared);
        return shared;
    }

private:
    TtyPipe() = default;
};


#endif //FICUS_TTYPIPE_HPP
