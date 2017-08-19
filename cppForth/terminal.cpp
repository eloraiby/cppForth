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
#include <cstdlib>

namespace Forth {
bool
Terminal::isInt(const SM::String& tok) {
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
Terminal::toInt32(const SM::String& tok) {
    uint32_t    pos = 0;
    uint32_t    val = 0;
    
    while( pos < tok.size() ) {
        val = val * 10 + (tok[pos] - '0');
        ++pos;
    }

    return val;
}

SM::String
Terminal::getToken() {
    SM::String ret;
    
    // remove white space
    while( stream()->peekChar() && IInputStream::isSpace(stream()->peekChar()) ) { stream()->getChar(); }
    
    // get token
    while( stream()->peekChar() && !IInputStream::isSpace(stream()->peekChar()) ) {
        ret += stream()->getChar();
    }

    return ret;
}

//
// TODO: this should be implemented in forth directly
//
void
Terminal::loadStream(IInputStream::Ptr strm) {
    streams_.push_back(strm);

    while( stream()->peekChar() && sig_.ty == Signal::NONE ) {
        SM::String tok = getToken();

        switch( stream()->getMode() ) {
        case IInputStream::Mode::EVAL:
            if( isInt(tok) ) {
                Value v(toInt32(tok));
                valueStack_.push_back(v);
            } else {
                if( vm_->nameToWord().find(tok) == vm_->nameToWord().end() ) {
                    char buff[MAX_BUFF] = {0};
                    sprintf(buff, "ERROR: word not found (%s)", tok.c_str());
                    emitSignal(Signal(Signal::EXCEPTION, pid_, ErrorCase::WORD_NOT_FOUND));
                } else {
                    runCall(vm_->nameToWord()[tok]);
                }
            }
            break;

        case IInputStream::Mode::COMPILE:
            if( isInt(tok) ) {
                vm_->emit(0);
                vm_->emit(Value(toInt32(tok)).u32);
            } else {
                if( vm_->nameToWord().find(tok) == vm_->nameToWord().end() ) {
                    char buff[MAX_BUFF] = {0};
                    sprintf(buff, "ERROR: word not found (%s)", tok.c_str());
                    emitSignal(Signal(Signal::EXCEPTION, pid_, ErrorCase::WORD_NOT_FOUND));
                } else {
                    uint32_t    word    = vm_->nameToWord()[tok];
                    if( vm_->functions()[word].isImmediate ) {
                        runCall(word);
                    } else {
                        vm_->emit(word);
                    }
                }
            }
            break;
        }
    }

    streams_.pop_back();
}

////////////////////////////////////////////////////////////////////////////////
// vm primitives
////////////////////////////////////////////////////////////////////////////////
void
Terminal::wordId(SM::VM::Process* proc) {
    Terminal* term = static_cast<Terminal*>(proc);
    SM::String name    = term->getToken();
    
    if( Terminal::isInt(name) ) {
        term->emitSignal(Signal(Signal::EXCEPTION, term->pid_, ErrorCase::INT_IS_NO_WORD));
        return;
    }

    if( term->vm_->nameToWord().find(name) == term->vm_->nameToWord().end() ) {
        char buff[MAX_BUFF] = {0};
        sprintf(buff, "ERROR: word not found (%s)", name.c_str());
        term->emitSignal(Signal(Signal::EXCEPTION, term->pid_, ErrorCase::WORD_NOT_FOUND));
        return;
    }
    
    uint32_t    wordId = term->vm_->nameToWord()[name];
    term->vm_->emit(0);
    term->vm_->emit(wordId);
}

void
Terminal::defineWord(SM::VM::Process* proc) {
    Terminal* term = static_cast<Terminal*>(proc);

    SM::String name    = term->getToken();

    if( Terminal::isInt(name) ) {
        term->emitSignal(Signal(Signal::EXCEPTION, term->pid_, ErrorCase::INT_IS_NO_WORD));
    } else {
        //
        // TODO:    do we want to allow forward declaration ?
        //          In this case, we should test to see if the functions[nameToWord[name]].start == -1 && .native == nullptr
        //          before setting the the old function to a value
        //
        uint32_t    wordId  = term->vm_->addNormalFunction(name);
        
        if( term->vm_->isVerboseDebugging() ) {
            fprintf(stderr, "%s [%d]\n", name.c_str(), wordId);
        }
        term->stream()->setMode(IInputStream::Mode::COMPILE);
    }
}

void
Terminal::immediate(SM::VM::Process* proc) {
    Terminal* term = static_cast<Terminal*>(proc);
    term->vm_->setFunctionAsImmediate(term->vm_->functions().size() - 1);
}

void
Terminal::setLocalCount(SM::VM::Process* proc) {
    Terminal* term = static_cast<Terminal*>(proc);
    SM::String tok  = term->getToken();

    if( !Terminal::isInt(tok) ) {
        term->emitSignal(Signal(Signal::EXCEPTION, term->pid_, ErrorCase::LOCAL_IS_NOT_INT));
        return;
    }
    uint32_t i = Terminal::toInt32(tok);
    term->vm_->setFunctionLocalCount(term->vm_->functions().size() - 1, i);
}

void
Terminal::endWord(SM::VM::Process* proc) {
    Terminal* term = static_cast<Terminal*>(proc);
    term->stream()->setMode(IInputStream::Mode::EVAL);
    term->vm_->emit(1);
}

void
Terminal::streamPeek(SM::VM::Process* proc) {
    Terminal* term = static_cast<Terminal*>(proc);
    // TODO: handle stream error (if it does not exist)
    Value v(static_cast<uint32_t>(term->stream()->peekChar()));
    term->pushValue(v);
}

void
Terminal::streamGetCH(SM::VM::Process* proc) {
    Terminal* term = static_cast<Terminal*>(proc);
    // TODO: handle stream error (does not exist)
    Value v(static_cast<uint32_t>(term->stream()->getChar()));
    term->pushValue(v);
}

void
Terminal::streamToken(SM::VM::Process* proc) {
    Terminal* term = static_cast<Terminal*>(proc);
    // TODO: when strings are ready
}

void
Terminal::see(SM::VM::Process* proc) {
    Terminal* term = static_cast<Terminal*>(proc);
    uint32_t    word    = term->vm_->nameToWord()[term->getToken()];
    fprintf(stdout, "[%d] : %s ", word, term->vm_->functions()[word].name.c_str());
    if( term->vm_->functions()[word].color == SM::VM::Function::Color::NATIVE ) {
         fprintf(stdout, " <native> ");
    } else {
        int32_t     curr    = term->vm_->functions()[word].body.interpreted.start;
        while( term->vm_->wordSegment()[curr] != 1 ) {
            if( term->vm_->wordSegment()[curr] == 0 ) {
                fprintf(stdout, "%d ", term->vm_->wordSegment()[++curr]);
            } else {
                fprintf(stdout, "@%d:%s ", curr, term->vm_->functions()[term->vm_->wordSegment()[curr]].name.c_str());
            }

            ++curr;
        }
    }

    if( term->vm_->functions()[word].isImmediate ) {
        fprintf(stdout, "immediate");
    }

    fprintf(stdout, "\n");
}



Terminal::Terminal(SM::VM* vm) : SM::VM::Process(nullptr, 0) {
    struct Primitive {
        const char*             name;
        SM::VM::NativeFunction  native;
        bool                    isImmediate;
    };

    static Primitive primitives[] = {
            { ":"           , Terminal::defineWord      , false },
            { "immediate"   , Terminal::immediate       , true  },
            { "locals"      , Terminal::setLocalCount   , true  },
            { ";"           , Terminal::endWord         , true  },
            { "'"           , Terminal::wordId          , true  },

            { "stream.peek" , Terminal::streamPeek      , false },
            { "stream.getch", Terminal::streamGetCH     , false },

            { "see"         , Terminal::see             , false },
    };

    vm_ = vm;

    for(Primitive p : primitives) {
        vm_->addNativeFunction(p.name, p.native, p.isImmediate);
    }
}

}   // namespace Forth