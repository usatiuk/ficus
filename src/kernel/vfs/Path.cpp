//
// Created by Stepan Usatiuk on 23.02.2024.
//

#include "Path.hpp"
Path StrToPath(const String &str) {
    if (str.length() == 0) return Path();
    Path   out;
    String buf;
    for (size_t i = 0; i < str.length(); i++) {
        if (str.c_str()[i] == '/') {
            if (buf.length() > 0)
                out.emplace_back(std::move(buf));
        } else {
            buf += str.c_str()[i];
        }
    }
    if (buf.length() > 0)
        out.emplace_back(std::move(buf));
    return out;
}
