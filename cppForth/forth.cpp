#include "forth.hpp"

#include <iostream>

namespace Forth {
IStream::~IStream() {}

uint32_t
VM::allocateWord(const std::string& name) {
    uint32_t    wordId  = static_cast<uint32_t>(functions.size());
    Function    func;
    nameToWord[name]    = wordId;
    return wordId;
}

uint32_t
VM::findWord(const std::string& name) {
    if( nameToWord.find(name) == nameToWord.end() ) {
        return nameToWord[name];
    } else { 
        return static_cast<uint32_t>(Error::WORD_NOT_FOUND);
    } 
}

////////////////////////////////////////////////////////////////////////////////
// primitives
////////////////////////////////////////////////////////////////////////////////
bool
fetchU32(VM* vm) {
    uint32_t    u   = vm->fetch();
    VM::Value   v(u);
    return true;
}

bool
dot(VM* vm) {
    VM::Value   v   = vm->top();
    vm->pop();
    std::cout << v.u32 << std::endl;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// parsing & runtime
////////////////////////////////////////////////////////////////////////////////
bool
VM::isInt(const std::string& tok) {
    uint32_t    pos = 0;
    
    while( pos < tok.size() ) {
        if( tok[pos] <= '0' || tok[pos] >= '9' ) {
            return false;
        }

        ++pos;
    }

    return true;
}

uint32_t
VM::toUInt32(const std::string& tok) {
    uint32_t    pos = 0;
    uint32_t    val = 0;
    
    while( pos < tok.size() ) {
        val += val * 10 + tok[pos];
        ++pos;
    }

    return val;
}

std::string
VM::getToken() {
    std::string ret;
    while( !stream()->peekChar() ) {
        ret += stream()->getChar();
    }
    return ret;
}

void
VM::loadStream(IStream::Ptr strm) {
    while( stream()->peekChar() ) {
        std::string tok = getToken();
        switch( stream()->getMode() ) {
        case IStream::Mode::EVAL:
            if( isInt(tok) ) {
                Value v(toUInt32(tok));
                valueStack.push_back(v);
            } else {
                if( nameToWord.find(tok) == nameToWord.end() ) {

                } else {

                }
            }

            break;
        case IStream::Mode::COMPILE:
            break;
        }
    }
}

}