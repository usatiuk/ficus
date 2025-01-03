//
// Created by Stepan Usatiuk on 26.04.2024.
//

#ifndef FBTTY_HPP
#define FBTTY_HPP

#include <PS2Keyboard.hpp>
#include <Tty.hpp>
#include <Vector.hpp>

class Framebuffer;
class FbTty : public Tty {
public:
             FbTty(Framebuffer *fb);
    virtual ~FbTty() = default;

    void putchar(char c) override;
    void putstr(const char *str) override;
    char readchar() override;

private:
    void draw_char(int col, int row);

    Framebuffer *_fb;

    int _cur_col = 0;
    int _cur_row = 0;

    int _max_row = 0;
    int _max_col = 0;

    Vector<Vector<char>> _buf;

    void next_col();
    void next_row();

    PS2Keyboard kbd;
};


#endif //FBTTY_HPP
