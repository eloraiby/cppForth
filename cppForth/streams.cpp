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

namespace Forth {

void
StdInStream::testAndFillBuffer() {
    if(pos >= buff.size()) {
        char tmp[8192] = {0};
        char* res = fgets(tmp, 8192, stdin);
        buff += res;
        buff += '\n';
    }
}

uint32_t
StdInStream::peekChar() {
    testAndFillBuffer();
    return buff[pos];
}

uint32_t
StdInStream::getChar() {
    testAndFillBuffer();
    return buff[pos++];
}
    
IStream::Mode
StdInStream::getMode() const {
    return mode;
}
    
void
StdInStream::setMode(IStream::Mode m) {
    mode = m;
}

StdInStream::~StdInStream() {
}

StdInStream::StdInStream() : mode(Mode::EVAL), pos(0) {}




StringStream::StringStream(const char* str) : mode(Mode::EVAL), pos(0), buff(str) {}

uint32_t
StringStream::peekChar() {
    if( pos == buff.size() ) {
        return 0;
    } else {
        return buff[pos];
    }
}

uint32_t
StringStream::getChar() {
    if( pos == buff.size() ) {
        return 0;
    } else {
        return buff[pos++];
    }
}
    
IStream::Mode
StringStream::getMode() const {
    return mode;
}
    
void
StringStream::setMode(Mode m) {
    mode = m;
}

StringStream::~StringStream() {
}



}   // namespace Forth
