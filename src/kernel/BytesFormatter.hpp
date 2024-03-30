//
// Created by Stepan Usatiuk on 22.03.2024.
//

#ifndef FICUS_BYTESFORMATTER_HPP
#define FICUS_BYTESFORMATTER_HPP

#include "String.hpp"

/// Utility class to format byte values according to their magnitude
class BytesFormatter {
public:
    /// Structure for returning the processed byte value
    struct BytesFormat {
        String number; ///< Number part of the value
        String prefix; ///< Unit of measure
    };

    /// Formats the bytes in BytesFormat format
    /// \param bytes Number of bytes
    /// \return      BytesFormat value
    static BytesFormat format(unsigned long long bytes);

    /// Formats the bytes into a string
    /// \param bytes Number of bytes
    /// \return      String, consisting of the scaled number and the unit of measure separated by a space
    static String formatStr(unsigned long long bytes);
};


#endif //FICUS_BYTESFORMATTER_HPP
