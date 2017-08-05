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
    Forth::IInputStream::Ptr coreStream(new Forth::StringStream(core.c_str()));
    vm->loadStream(coreStream);

    Forth::IInputStream::Ptr strm(new Forth::StdInStream());
    vm->loadStream(strm);

    delete vm;

    return 0;
}

