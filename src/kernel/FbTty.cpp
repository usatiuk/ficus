//
// Created by Stepan Usatiuk on 26.04.2024.
//

#include "FbTty.hpp"
#include "Fonts.hpp"

#include <Framebuffer.hpp>

FbTty::FbTty(Framebuffer *fb) : _fb(fb) {
    _max_col = _fb->dimensions().x / 8;
    _max_row = _fb->dimensions().y / 16;
    _buf.resize(_max_col);
    for (auto &b: _buf) {
        b.resize(_max_row);
        for (int i = 0; i < _max_row; i++)
            b[i] = ' ';
    }
}

void FbTty::draw_char(int col, int row) {
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 16; y++) {
            uint32_t color = (fonts_Terminess_Powerline[_buf[col][row]][y] & (1 << (8 - x))) ? 0xFFFFFF : 0;
            _fb->set(col * 8 + x, row * 16 + y, color);
        }
    }
}

void FbTty::putchar(char c) {
    if (c == '\n') {
        next_row();
        return;
    }

    _buf[_cur_col][_cur_row] = c;

    draw_char(_cur_col, _cur_row);

    next_col();
}
void FbTty::putstr(const char *str) {
    while (*str != 0) {
        putchar(*str);
        str++;
    }
}
char FbTty::readchar() {
    char r = kbd.readchar();
    if (r != '\r')
        putchar(r);
    return r;
}
void FbTty::next_col() {
    _cur_col++;
    _cur_col = _cur_col % _max_col;
    if (_cur_col == 0) {
        next_row();
    } else {
        _buf[_cur_col][_cur_row] = ' ';
        draw_char(_cur_col, _cur_row);
    }
}
void FbTty::next_row() {
    _cur_col = 0;
    _cur_row++;
    _cur_row = _cur_row % _max_row;
    for (int i = 0; i < _max_col; i++) {
        _buf[i][_cur_row] = ' ';
        draw_char(i, _cur_row);
    }
}
