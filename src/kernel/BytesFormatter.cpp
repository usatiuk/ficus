//
// Created by Stepan Usatiuk on 22.03.2024.
//

#include "BytesFormatter.hpp"

static void print2dec(String &out, uint64_t what) {
    uint64_t after_dot = what % 100;
    what /= 100;
    out += what;
    out += ".";
    out += after_dot;
}

BytesFormatter::BytesFormat BytesFormatter::format(unsigned long long int bytes) {
    String outNum;

    if (bytes > 1024ULL * 1024 * 1024 * 1024) {
        print2dec(outNum, bytes / (1024ULL * 1024ULL * 1024ULL * 10ULL));
        return {std::move(outNum), "TiB"};
    }
    if (bytes > 1024ULL * 1024 * 1024) {
        print2dec(outNum, bytes / (1024ULL * 1024ULL * 10ULL));
        return {std::move(outNum), "GiB"};
    }
    if (bytes > 1024ULL * 1024) {
        print2dec(outNum, bytes / (1024ULL * 10ULL));
        return {std::move(outNum), "MiB"};
    }
    if (bytes > 1024ULL) {
        print2dec(outNum, bytes / (10ULL));
        return {std::move(outNum), "KiB"};
    }
    outNum += bytes;
    return {std::move(outNum), "Bytes"};
}

String BytesFormatter::formatStr(unsigned long long int bytes) {
    auto fmt = format(bytes);
    String out;
    out += fmt.number;
    out += " ";
    out += fmt.prefix;
    return out;
}