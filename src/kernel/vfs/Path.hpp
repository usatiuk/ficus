//
// Created by Stepan Usatiuk on 23.02.2024.
//

#ifndef FICUS_PATH_HPP
#define FICUS_PATH_HPP

#include "List.hpp"
#include "String.hpp"
#include "Vector.hpp"

using Path = Vector<String>;

Path StrToPath(const String &str);

#endif //FICUS_PATH_HPP
