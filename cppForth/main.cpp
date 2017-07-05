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

#include <iostream>
#include "forth.hpp"

struct StdStream : public Forth::IStream {
    void
    testAndFillBuffer() {
        if(pos >= buff.size()) {
            std::string tmp;
            std::cin >> tmp;
            tmp += '\n';
            buff += tmp;
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
    std::string buff;
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
    std::string buff;
};

const char
core[] =
    ": i32>w ' lit.i32 >w >w ;\n" \
    ": begin w.p ; immediate\n" \
    ": until i32>w ' ?branch >w ; immediate\n" \
    ": (  begin stream.getch 41 =/= until ; immediate\n" \
    ": \\ begin stream.getch 13 =/= until ; immediate\n" \
    ": if ( cond -- )\n" \
    "   w.p 3 +\n" \
    "   i32>w \\ 0 1\n" \
    "   ' ?branch >w \\ 2\n" \
    "   w.p\n" \
    "   0 i32>w \\ 3 4\n"\
    "   ' branch >w \\ 5\n" \
    "; immediate\n" \
    ": then\n" \
    "   w.p swap 1 + !\n" \
    "; immediate\n" \
    ": else ; immediate\n" \
    ": *2 2 * ;\n" \
    ": /2 2 / ;\n" \
    ": +1 1 + ;\n" \
    ": -1 1 - ;\n"
    ;

int
main(int argc, char* argv[]) {
    Forth::VM*  vm  = new Forth::VM();

    Forth::IStream::Ptr coreStream(new StringStream(core));
    vm->loadStream(coreStream);

    Forth::IStream::Ptr strm(new StdStream());
    vm->loadStream(strm);

    return 0;
}
