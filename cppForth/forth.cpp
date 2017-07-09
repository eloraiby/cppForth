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
#include <cstdio>

namespace Forth {

IStream::~IStream() {}

int32_t
VM::findWord(const std::string& name) {
    if( nameToWord.find(name) == nameToWord.end() ) {
        return nameToWord[name];
    } else { 
        return static_cast<uint32_t>(-1);
    } 
}

uint32_t
VM::addNativeFunction(const std::string& name, NativeFunction native, bool isImmediate) {
    uint32_t    wordId  = static_cast<uint32_t>(functions.size());
    Function    func;

    func.name   = name;

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
    uint32_t    word    = words[wp];
        
    if( word > functions.size() ) {
        throwException(ErrorCase::WORD_ID_OUT_OF_RANGE, "ERROR: word outside code segment");
        return;
    }

    if( verboseDebugging ) {
        std::cout << "    @" << wp << " -- " << functions[word].name;
        if( word == 0 ) {
            std::cout << " " << words[wp + 1];
        }
        std::cout << std::endl;
    }

    if( functions[word].native ) {
        functions[word].native(this);
        ++wp;
    } else {
        if(  functions[word].start == -1 ) {
            throwException(ErrorCase::WORD_NOT_DEFINED, "ERROR: word not defined");
            return;
        } else {
            if( verboseDebugging ) {
                std::cout << functions[word].name << ":" << std::endl;
            }
            setCall(word);
        }
    }
}

void
VM::throwException(ErrorCase err, const std::string& str) {
    exceptionStack.push_back(Error(err, str));

    std::cerr << str << std::endl;

    for( int i = callStack.size() - 1; i >= 0 ; --i ) {
        std::cerr << "\t@[" << callStack[i] << "] - " << functions[callStack[i]].name << std::endl;
    }

    // TODO: switch to debug stream (debugging stream)
    // popStream();
    stream()->setMode(IStream::Mode::EVAL);
}

void
VM::runCall(uint32_t word) {
    size_t    startExceptionSize = exceptionStack.size();

    if( word > functions.size() ) {
        throwException(ErrorCase::WORD_ID_OUT_OF_RANGE, "ERROR: word outside code segment");
        return;
    }

    // IF verbose debugging AND IF function id exists
    if( verboseDebugging ) {
        std::cout << functions[word].name << ":" << std::endl;
    }

    if( functions[word].native ) {
        functions[word].native(this);
    } else {
        uint32_t    rsPos   = returnStack.size();

        setCall(word);

        while( returnStack.size() != rsPos && exceptionStack.size() <= startExceptionSize ) {
            step();
        }
    }
}

void
VM::loadStream(IStream::Ptr strm) {
    streams.push_back(strm);
    size_t    startExceptionSize = exceptionStack.size();

    while( stream()->peekChar() && exceptionStack.size() <= startExceptionSize ) {
        std::string tok = getToken();

        switch( stream()->getMode() ) {
        case IStream::Mode::EVAL:
            if( isInt(tok) ) {
                Value v(toInt32(tok));
                valueStack.push_back(v);
            } else {
                if( nameToWord.find(tok) == nameToWord.end() ) {
                    char buff[MAX_BUFF] = {0};
                    sprintf(buff, "ERROR: word not found (%s)", tok.c_str());
                    throwException(ErrorCase::WORD_NOT_FOUND, buff);
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
                    char buff[MAX_BUFF] = {0};
                    sprintf(buff, "ERROR: word not found (%s)", tok.c_str());
                    throwException(ErrorCase::WORD_NOT_FOUND, buff);
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

VM::VM() : wp(0), verboseDebugging(false) {
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
        { "#"           , Primitives::callIndirect  , false },
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
        { "deb.set"     , Primitives::setDebugMode  , false },

    };

    for(Primitive p : primitives) {
        addNativeFunction(p.name, p.native, p.isImmediate);
    }
}



}
