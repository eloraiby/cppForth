/* 
** Copyright (c) 2017 Wael El Oraiby.
** 
** This program is free software: you can redistribute it and/or modify  
** it under the terms of the GNU Lesser General Public License as   
** published by the Free Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but 
** WITHOUT ANY WARRANTY; without even the implied warranty of 
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
** Lesser General Lesser Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "forth.hpp"

#include <cstdio>

struct StdStream : public Forth::IStream {
    void
    testAndFillBuffer() {
        if(pos >= buff.size()) {
            char tmp[8192] = {0};
            scanf("%s", tmp);
            buff += tmp;
            buff += '\n';
        }
    }

    virtual uint32_t
    peekChar() override {
        testAndFillBuffer();
        return buff[pos];
    }

    virtual uint32_t
    getChar() override {
        testAndFillBuffer();
        return buff[pos++];
    }
    
    virtual Mode
    getMode() const override {
        return mode;
    }
    
    virtual void
    setMode(Mode m) override {
        mode = m;
    }

    ~StdStream()    override {
    }

    StdStream() : mode(Mode::EVAL), pos(0) {}


    Mode        mode;
    uint32_t    pos;
    Forth::String   buff;
};

struct StringStream : public Forth::IStream {

    StringStream(const char* str) : mode(Mode::EVAL), pos(0), buff(str) {}

    virtual uint32_t
    peekChar() override {
        if( pos == buff.size() ) {
            return 0;
        } else {
            return buff[pos];
        }
    }

    virtual uint32_t
    getChar() override {
        if( pos == buff.size() ) {
            return 0;
        } else {
            return buff[pos++];
        }
    }
    
    virtual Mode
    getMode() const override {
        return mode;
    }
    
    virtual void
    setMode(Mode m) override {
        mode = m;
    }

    ~StringStream()    override {
    }


    Mode        mode;
    uint32_t    pos;
    Forth::String buff;
};

Forth::String
readFile(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if( f == nullptr ) {
        return "";
    }

    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buff = new char[fsize + 1];
    fread(buff, 1, fsize, f);
    buff[fsize] = '\0';

    fclose(f);

    Forth::String ret(buff);
    delete[] buff;

    return ret;
}

int
main(int argc, char* argv[]) {
    Forth::VM*  vm  = new Forth::VM();

    Forth::String core    = readFile("bootstrap.f");
    Forth::IStream::Ptr coreStream(new StringStream(core.c_str()));
    vm->loadStream(coreStream);

    Forth::IStream::Ptr strm(new StdStream());
    vm->loadStream(strm);

    delete vm;

    return 0;
}

