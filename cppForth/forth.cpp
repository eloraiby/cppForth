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

namespace Forth {

IInputStream::~IInputStream() {}

int32_t
VM::findWord(const String& name) {
    if( nameToWord.find(name) == nameToWord.end() ) {
        return nameToWord[name];
    } else { 
        return static_cast<uint32_t>(-1);
    } 
}

uint32_t
VM::addNativeFunction(const String& name, NativeFunction native, bool isImmediate) {
    uint32_t    wordId  = static_cast<uint32_t>(functions.size());
    Function    func;

    func.name   = name;

    func.body.native = native;
    func.isImmediate    = isImmediate;
    functions.push_back(func);
    nameToWord[name]    = wordId;
    return wordId;
}



////////////////////////////////////////////////////////////////////////////////
// runtime
////////////////////////////////////////////////////////////////////////////////

void
VM::Process::step() {
    uint32_t    word    = vm_->wordSegment[wp_];
        
    if( word > vm_->functions.size() ) {
        emitSignal(VM::Process::Signal(VM::Process::Signal::WORD_ID_OUT_OF_RANGE, pid_, 0));
        return;
    }

    if( vm_->verboseDebugging ) {
        fprintf(stdout, "    @%d -- %s", wp_, vm_->functions[word].name.c_str());
        if( word == 0 ) {
            fprintf(stdout, " %d", vm_->wordSegment[wp_ + 1]);
        }
        fprintf(stdout, "\n");
    }

    if( vm_->functions[word].color == VM::Function::Color::NATIVE ) {
        vm_->functions[word].body.native(this);
        ++wp_;
    } else {
        if( vm_->functions[word].body.interpreted.start == -1 ) {
            emitSignal(VM::Process::Signal(VM::Process::Signal::WORD_NOT_IMPLEMENTED, pid_, 0));
            return;
        } else {
            if( vm_->verboseDebugging ) {
                fprintf(stdout, "%s:\n", vm_->functions[word].name.c_str());
            }
            setCall(word);
        }
    }
}

void
VM::Process::emitSignal(const VM::Process::Signal& sig) {
    sig_    = sig;

    for( int i = returnStack_.size() - 1; i >= 0 ; --i ) {
        fprintf(stderr, "\t@[%d] - %s\n", returnStack_[i].word, vm_->functions[returnStack_[i].word].name.c_str());
    }
}

void
VM::Process::runCall(uint32_t word) {

    if( word > vm_->functions.size() ) {
        emitSignal(VM::Process::Signal(VM::Process::Signal::WORD_ID_OUT_OF_RANGE, pid_, 0));
        return;
    }

    // IF verbose debugging AND IF function id exists
    if( vm_->verboseDebugging ) {
        fprintf(stdout, "%s:\n", vm_->functions[word].name.c_str());
    }

    if( vm_->functions[word].color == VM::Function::Color::NATIVE && sig_.ty == Signal::NONE ) {
        vm_->functions[word].body.native(this);
    } else {
        uint32_t    rsPos   = returnStack_.size();

        setCall(word);

        while( returnStack_.size() != rsPos && sig_.ty == Signal::NONE ) {
            step();
        }
    }
}


VM::Process::Process(uint32_t pid) :  sig_(Signal(VM::Process::Signal::NONE, 0, 0)), pid_(pid), wp_(0), lp_(0) {}

VM::VM() : verboseDebugging(false) {
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
        { "#"           , Primitives::callIndirect  , false },
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
        { "w>"          , Primitives::emitWord      , false },
        { "cd>"         , Primitives::emitConstData , false },
        { "e>"          , Primitives::emitException , false },


        
        { "=="          , Primitives::ieq           , false },
        { "=/="         , Primitives::ineq          , false },
        { ">"           , Primitives::igt           , false },
        { "<"           , Primitives::ilt           , false },
        { ">="          , Primitives::igeq          , false },
        { "<="          , Primitives::ileq          , false },
        { "not"         , Primitives::notBW         , false },
        { "and"         , Primitives::andBW         , false },
        { "or"          , Primitives::orBW          , false },

        { "v&"          , Primitives::vsPtr         , false },
        { "r&"          , Primitives::rsPtr         , false },
        { "w&"          , Primitives::wsPtr         , false },
        { "cd&"         , Primitives::cdsPtr        , false },
        { "@"           , Primitives::vsFetch       , false },
        { "r@"          , Primitives::rsFetch       , false },
        { "w@"          , Primitives::wsFetch       , false },
        { "l@"          , Primitives::lsFetch       , false },
        { "cd@"         , Primitives::cdsFetch      , false },
        { "!"           , Primitives::vsStore       , false },
        { "w!"          , Primitives::wsStore       , false },
        { "l!"          , Primitives::lsStore       , false },
        { "cd!"         , Primitives::cdsStore      , false },
        
        { "bye"         , Primitives::bye           , false },
        { "exit"        , Primitives::exit          , false },

        { ".s"          , Primitives::showValueStack, false },
        { "deb.set"     , Primitives::setDebugMode  , false },

        { "'"           , Terminal::wordId          , true  },
        { ":"           , Terminal::defineWord      , false },
        { ";"           , Terminal::endWord         , true  },
        { "immediate"   , Terminal::immediate       , true  },
        { "locals"      , Terminal::setLocalCount   , true  },
        { "stream.peek" , Terminal::streamPeek      , false },
        { "stream.getch", Terminal::streamGetCH     , false },
        { "see"         , Terminal::see             , false },
    };

    for(Primitive p : primitives) {
        addNativeFunction(p.name, p.native, p.isImmediate);
    }
}



}
