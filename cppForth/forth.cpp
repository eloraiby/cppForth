#include "forth.hpp"

#include <iostream>

namespace Forth {
IStream::~IStream() {}

VM::State
VM::defineWord(VM* vm) {
    uint32_t    wordId  = static_cast<uint32_t>(vm->functions.size());

    std::string name    = vm->getToken();

    if( VM::isInt(name) ) {
        return VM::State::INT_IS_NO_WORD;
    } else {
        Function    func;
        func.start  = vm->words.size();



        return VM::State::NO_ERROR;
    }
}

int32_t
VM::findWord(const std::string& name) {
    if( nameToWord.find(name) == nameToWord.end() ) {
        return nameToWord[name];
    } else { 
        return static_cast<uint32_t>(State::WORD_NOT_FOUND);
    } 
}

uint32_t
VM::addNativeFunction(const std::string& name, NativeFunction native) {
    uint32_t    wordId  = static_cast<uint32_t>(functions.size());
    Function    func;
    func.start  = -1;
    func.native = native;
    functions.push_back(func);
    nameToWord[name]    = wordId;
    return wordId;
}

////////////////////////////////////////////////////////////////////////////////
// primitives
////////////////////////////////////////////////////////////////////////////////
VM::State
fetchI32(VM* vm) {
    int32_t    u   = vm->fetch();
    VM::Value   v(u);
    return VM::State::NO_ERROR;
}

VM::State
dot(VM* vm) {
    VM::Value   v   = vm->top();
    vm->pop();
    std::cout << v.i32 << std::endl;
    return VM::State::NO_ERROR;
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

int32_t
VM::toInt32(const std::string& tok) {
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
    
    // remove white space
    while( !stream()->peekChar() && !IStream::isSpace(stream()->peekChar()) ) { stream()->getChar(); }
    
    // get token
    while( !stream()->peekChar() && !IStream::isSpace(stream()->peekChar()) ) {
        ret += stream()->getChar();
    }

    return ret;
}

void
VM::step() {
    if( state == State::NO_ERROR ) {
        uint32_t    word    = words[wp];
        
        if( word > functions.size() ) {
            state = State::WORD_NOT_FOUND;
            return;
        }

        if( functions[word].native ) {
            state = functions[word].native(this);
        } else {
            if(  functions[word].start == 0 ) {
                state = State::WORD_NOT_DEFINED;
                return;
            } else {
                setCall(word);
            }
        }
    }
}

void
VM::runCall(uint32_t word) {
    
    if( state != State::NO_ERROR ) {
        return;
    }

    if( word > functions.size() ) {
        state = State::WORD_NOT_FOUND;
        return;
    }

    if( functions[word].native ) {
        functions[word].native(this);
    } else {
        uint32_t    rsPos   = returnStack.size();

        setCall(word);

        while( returnStack.size() != rsPos && state == State::NO_ERROR ) {
            step();
        }
    }
}

void
VM::initPrimitives() {

}

void
VM::loadStream(IStream::Ptr strm) {
    while( stream()->peekChar() && state == State::NO_ERROR ) {
        std::string tok = getToken();

        switch( stream()->getMode() ) {
        case IStream::Mode::EVAL:
            if( isInt(tok) ) {
                Value v(toInt32(tok));
                valueStack.push_back(v);
            } else {
                if( nameToWord.find(tok) == nameToWord.end() ) {
                    state = State::WORD_NOT_FOUND;
                } else {
                    runCall(nameToWord[tok]);
                }
            }
            break;

        case IStream::Mode::COMPILE:
            if( isInt(tok) ) {
                words.push_back(0);
                words.push_back(toInt32(tok));
            } else {
                if( nameToWord.find(tok) == nameToWord.end() ) {
                    state = State::WORD_NOT_FOUND;
                } else {
                    runCall(nameToWord[tok]);
                }
            }
            break;
        }
    }
}

}