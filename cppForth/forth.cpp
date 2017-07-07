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

#include <iostream>

namespace Forth {

IStream::~IStream() {}

int32_t
VM::findWord(const std::string& name) {
    if( nameToWord.find(name) == nameToWord.end() ) {
        return nameToWord[name];
    } else { 
        return static_cast<uint32_t>(State::WORD_NOT_FOUND);
    } 
}

uint32_t
VM::addNativeFunction(const std::string& name, NativeFunction native, bool isImmediate) {
    uint32_t    wordId  = static_cast<uint32_t>(functions.size());
    Function    func;
#ifdef _DEBUG
    func.name   = name;
#endif
    func.start  = -1;
    func.native = native;
    func.isImmediate    = isImmediate;
    functions.push_back(func);
    nameToWord[name]    = wordId;
    return wordId;
}



////////////////////////////////////////////////////////////////////////////////
// parsing & runtime
////////////////////////////////////////////////////////////////////////////////
bool
VM::isInt(const std::string& tok) {
    uint32_t    pos = 0;
    
    while( pos < tok.size() ) {
        if( tok[pos] < '0' || tok[pos] > '9' ) {
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
        val = val * 10 + (tok[pos] - '0');
        ++pos;
    }

    return val;
}

std::string
VM::getToken() {
    std::string ret;
    
    // remove white space
    while( stream()->peekChar() && IStream::isSpace(stream()->peekChar()) ) { stream()->getChar(); }
    
    // get token
    while( stream()->peekChar() && !IStream::isSpace(stream()->peekChar()) ) {
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

#ifdef _DEBUG
        std::cout << "    @" << wp << " -- " << functions[word].name;
        if( word == 0 ) {
            std::cout << " " << words[wp + 1];
        }
        std::cout << std::endl;
#endif
        if( functions[word].native ) {
            callStack.push_back(word);
            state = functions[word].native(this);
            callStack.pop_back();
            ++wp;
        } else {
            if(  functions[word].start == -1 ) {
                state = State::WORD_NOT_DEFINED;
                return;
            } else {
#ifdef _DEBUG
                std::cout << functions[word].name << ":" << std::endl;
#endif
                setCall(word);
            }
        }
    }
}

void
VM::runCall(uint32_t word) {
    
#ifdef _DEBUG
    std::cout << functions[word].name << ":" << std::endl;
#endif

    if( state != State::NO_ERROR ) {
        return;
    }

    if( word > functions.size() ) {
        std::cerr << "ERROR: word outside code segement" << std::endl;
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
VM::loadStream(IStream::Ptr strm) {
    streams.push_back(strm);

    while( stream()->peekChar() && state == State::NO_ERROR ) {
        std::string tok = getToken();

        switch( stream()->getMode() ) {
        case IStream::Mode::EVAL:
            if( isInt(tok) ) {
                Value v(toInt32(tok));
                valueStack.push_back(v);
            } else {
                if( nameToWord.find(tok) == nameToWord.end() ) {
                    std::cerr << "ERROR: word not found (" << tok << ")" << std::endl;
                    state = State::WORD_NOT_FOUND;
                } else {
                    runCall(nameToWord[tok]);
                }
            }
            break;

        case IStream::Mode::COMPILE:
            if( isInt(tok) ) {
                emit(0);
                emit(Value(toInt32(tok)).u32);
            } else {
                if( nameToWord.find(tok) == nameToWord.end() ) {
                    std::cerr << "ERROR: word not found (" << tok << ")" << std::endl;
                    state = State::WORD_NOT_FOUND;
                } else {
                    uint32_t    word    = nameToWord[tok];
                    if( functions[word].isImmediate ) {
                        runCall(word);
                    } else {
                        emit(word);
                    }
                }
            }
            break;
        }
    }

    streams.pop_back();
}

VM::VM() : wp(0), state(VM::State::NO_ERROR) {
    initPrimitives();
}

////////////////////////////////////////////////////////////////////////////////
// primitives
////////////////////////////////////////////////////////////////////////////////
void
VM::initPrimitives() {
    struct Primitive {
        const char*     name;
        NativeFunction  native;
        bool            isImmediate;
    };

    Primitive primitives[] = {
        { "lit.i32"     , Primitives::fetchInt32    , false },
        { "return"      , Primitives::returnWord    , false },
        { "'"           , Primitives::wordId        , true  },
        { ":"           , Primitives::defineWord    , false },
        { "immediate"   , Primitives::immediate     , true  },
        { "."           , Primitives::printInt32    , false },
        { ".c"          , Primitives::printChar     , false },
        { "+"           , Primitives::addInt32      , false },
        { "-"           , Primitives::subInt32      , false },
        { "*"           , Primitives::mulInt32      , false },
        { "/"           , Primitives::divInt32      , false },
        { "%"           , Primitives::modInt32      , false },
        { "branch"      , Primitives::branch        , false },  // ( addr -- )
        { "?branch"     , Primitives::branchIf      , false },  // ( cond addr -- )
        { "dup"         , Primitives::dup           , false },
        { "drop"        , Primitives::drop          , false },
        { "swap"        , Primitives::swap          , false },
        { "code.size"   , Primitives::codeSize      , false },
        { ";"           , Primitives::endWord       , true  },
        { ">r"          , Primitives::emitReturn    , false },
        { ">w"          , Primitives::emitWord      , false },
        { ">cd"         , Primitives::emitConstData , false },
        { ">e"          , Primitives::emitException , false },

        { "stream.peek" , Primitives::streamPeek    , false },
        { "stream.getch", Primitives::streamGetCH   , false },
        
        { "=="          , Primitives::ieq           , false },
        { "=/="         , Primitives::ineq          , false },
        { ">"           , Primitives::igt           , false },
        { "<"           , Primitives::ilt           , false },
        { ">="          , Primitives::igeq          , false },
        { "<="          , Primitives::ileq          , false },
        { "not"         , Primitives::notBW         , false },
        { "and"         , Primitives::andBW         , false },
        { "or"          , Primitives::orBW          , false },

        { "v.p"         , Primitives::vsPtr         , false },
        { "r.p"         , Primitives::rsPtr         , false },
        { "w.p"         , Primitives::wsPtr         , false },
        { "cd.p"        , Primitives::cdsPtr        , false },
        { "e.p"         , Primitives::esPtr         , false },
        { "@"           , Primitives::vsFetch       , false },
        { "@r"          , Primitives::rsFetch       , false },
        { "@w"          , Primitives::wsFetch       , false },
        { "@cd"         , Primitives::cdsFetch      , false },
        { "@e"          , Primitives::esFetch       , false },
        { "!"           , Primitives::vsStore       , false },
        { "!r"          , Primitives::rsStore       , false },
        { "!w"          , Primitives::wsStore       , false },
        { "!cd"         , Primitives::cdsStore      , false },
        { "!e"          , Primitives::esStore       , false },

        { "exit"        , Primitives::exit          , false },

        { ".s"          , Primitives::showValueStack, false },
        { "see"         , Primitives::see           , false },

    };

    for(Primitive p : primitives) {
        addNativeFunction(p.name, p.native, p.isImmediate);
    }
}



}
