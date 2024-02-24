//
// Created by Stepan Usatiuk on 23.02.2024.
//

#ifndef OS2_PATH_HPP
#define OS2_PATH_HPP

#include "List.hpp"
#include "String.hpp"
#include "Vector.hpp"

using Path = Vector<String>;

Path StrToPath(const String &str);

#endif//OS2_PATH_HPP
