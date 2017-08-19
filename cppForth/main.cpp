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
#include <stdio.h>

SM::String
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

    SM::String ret(buff);
    delete[] buff;

    return ret;
}

int
main(int argc, char* argv[]) {
    SM::VM*  vm  = new SM::VM();

    SM::String core    = readFile("bootstrap.f");
    Forth::IInputStream::Ptr coreStream(new Forth::StringStream(core.c_str()));
    Forth::Terminal::Ptr    term(new Forth::Terminal(vm));
    term->loadStream(coreStream);

    Forth::IInputStream::Ptr strm(new Forth::StdInStream());
    term->loadStream(strm);

    delete vm;

    return 0;
}

