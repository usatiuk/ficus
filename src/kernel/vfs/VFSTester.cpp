//
// Created by Stepan Usatiuk on 23.02.2024.
//

#include "VFSTester.hpp"

#include "File.hpp"
#include "VFSApi.hpp"

void VFSTester::test() {
    VFSApi::mkdir(StrToPath("/home/hello"));
    VFSApi::mkdir(StrToPath("/home/hello/hellod2"));
    VFSApi::touch(StrToPath("/home/hellof"));
    VFSApi::touch(StrToPath("/home/hello/f2"));

    FDT::FD a = VFSApi::open(StrToPath("/home/hello"));
    FDT::FD b = VFSApi::open(StrToPath("/home/hello/hellod2"));
    FDT::FD c = VFSApi::open(StrToPath("/home/hellof"));
    FDT::FD d = VFSApi::open(StrToPath("/home/hello/f2"));
    {
        String t("hello wooooorld");
        File  *cf = VFSApi::get(c);
        cf->write(t.c_str(), t.length() + 1);
    }
    assert(a != -1);
    assert(b != -1);
    assert(c != -1);
    assert(d != -1);
    VFSApi::close(a);
    VFSApi::close(b);
    VFSApi::close(c);
    VFSApi::close(d);

    c = VFSApi::open(StrToPath("/home/hellof"));
    assert(c != -1);
    {
        String t("aaaaaaaaaaaaaaaaaaaa");
        File  *cf = VFSApi::get(c);
        cf->read(t.data(), cf->size());
        assert(t == "hello wooooorld");
    }
    {
        String t("aaaaaaaaaaaaaaaaaaaa");
        File  *cf = VFSApi::get(c);
        cf->seek(0);
        cf->read(t.data(), 9);
        cf->read(t.data() + 9, cf->size() - 9);
        assert(t == "hello wooooorld");
    }
}
