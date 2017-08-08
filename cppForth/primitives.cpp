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
#define VS_CHECK() \
    if( proc->valueStack_.size() == 0 ) { /* vm->throwException(VM::ErrorCase::VS_UNDERFLOW, "value stack underflow");*/ return; } \

#define VS_POP(V)   \
    if( proc->valueStack_.size() == 0 ) { /* proc->throwException(VM::ErrorCase::VS_UNDERFLOW, "value stack underflow");*/ return; } \
    VM::Value   V = proc->topValue(); \
    proc->popValue()

void
Primitives::fetchInt32(VM::Process* proc) {
    int32_t    u   = proc->fetch();
    VM::Value   v(u);
    proc->pushValue(v);
}

void
Primitives::returnWord(VM::Process* proc) {
    proc->setRet();
}

void
Primitives::wordId(VM::Process* proc) {
    String name    = proc->getToken();
    
    if( vm->isInt(name) ) {
        vm->throwException(VM::ErrorCase::INT_IS_NO_WORD, "Int was used as a word definition");
        return;
    }

    if( vm->nameToWord.find(name) == vm->nameToWord.end() ) {
        char buff[MAX_BUFF] = {0};
        sprintf(buff, "ERROR: word not found (%s)", name.c_str());
        vm->throwException(VM::ErrorCase::WORD_NOT_FOUND, buff);
        return;
    }
    
    uint32_t    wordId = vm->nameToWord[name];
    vm->emit(0);
    vm->emit(wordId);
}

void
Primitives::callIndirect(VM::Process* proc) {
    VS_POP(u);
    proc->setCall(u.u32);
    --proc->wp;   // once outside the native, wp will get incremented, so decrement to stay at the start of the word
}

void
Primitives::printInt32(VM::Process* proc) {
    VS_POP(v);
    fprintf(stdout, "%d\n", v.i32);
}

void
Primitives::printChar(VM::Process* proc) {
    VS_POP(v);
    fprintf(stdout, "%c", static_cast<char>(v.i32));
}

void
Primitives::defineWord(VM::Process* proc) {

    String name    = vm->getToken();

    if( VM::isInt(name) ) {
        vm->throwException(VM::ErrorCase::INT_IS_NO_WORD, "Int was used as a word definition");
    } else {
        //
        // TODO:    do we want to allow forward declaration ?
        //          In this case, we should test to see if the functions[nameToWord[name]].start == -1 && .native == nullptr
        //          before setting the the old function to a value
        //
        uint32_t    wordId  = static_cast<uint32_t>(vm->functions.size());

        VM::Function    func;

        func.name   = name;

        func.start  = vm->wordSegment.size();
        vm->functions.push_back(func);

        vm->nameToWord[name]    = wordId;
        
        vm->stream()->setMode(IInputStream::Mode::COMPILE);
    }
}

void
Primitives::immediate(VM::Process* proc) {
    proc->vm_->functions[proc->vm_->functions.size() - 1].isImmediate = true;
}

void
Primitives::setLocalCount(VM::Process* proc) {
    String tok  = vm->getToken();

    if( !VM::isInt(tok) ) {
        vm->throwException(VM::ErrorCase::LOCAL_IS_NOT_INT, "a local should be an integer");
        return;
    }

    vm->functions[vm->functions.size() - 1].localCount = VM::toInt32(tok);
}

void
Primitives::addInt32(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Value(a.i32 + b.i32));
}

void
Primitives::subInt32(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Value(a.i32 - b.i32));
}

void
Primitives::mulInt32(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Value(a.i32 * b.i32));
}

void
Primitives::divInt32(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Value(a.i32 / b.i32));
}

void
Primitives::modInt32(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Value(a.i32 % b.i32));
}

void
Primitives::branch(VM::Process* proc) {
    VS_POP(addr);
    proc->setBranch(addr.i32 - 1);
}

void
Primitives::branchIf(VM::Process* proc) {
    VS_POP(addr);
    VS_POP(cond);

    if( cond.i32 != 0 ) {
        proc->setBranch(addr.i32 - 1);
    }
}

void
Primitives::dup(VM::Process* proc) {
    VS_CHECK()
    VM::Value val   = proc->topValue();
    proc->pushValue(val);
}

void
Primitives::drop(VM::Process* proc) {
    VS_CHECK()
    proc->popValue();
}

void
Primitives::swap(VM::Process* proc) {
    VS_POP(v0);
    VS_POP(v1);

    proc->pushValue(v0);
    proc->pushValue(v1);
}

void
Primitives::codeSize(VM::Process* proc) {
    VM::Value v(static_cast<int32_t>(proc->vm_->wordSegment.size()));
    proc->pushValue(v);
}

void
Primitives::endWord(VM::Process* proc) {
    vm->stream()->setMode(IInputStream::Mode::EVAL);
    vm->emit(1);
}

void
Primitives::emitWord(VM::Process* proc) {
    VS_POP(v);
    proc->vm_->emit(v.u32);
}

void
Primitives::emitConstData(VM::Process* proc) {
    VS_POP(v);
    proc->vm_->constDataSegment.push_back(v);
}

void
Primitives::emitException(VM::Process* proc) {
    VS_POP(v);
    proc->exceptionStack.push_back(VM::Error(static_cast<VM::ErrorCase>(v.i32), ""));
}

void
Primitives::streamPeek(VM::Process* proc) {
    // TODO: handle stream error (does not exist)
    VM::Value v(static_cast<uint32_t>(vm->stream()->peekChar()));
    vm->push(v);
}

void
Primitives::streamGetCH(VM::Process* proc) {
    // TODO: handle stream error (does not exist)
    VM::Value v(static_cast<uint32_t>(vm->stream()->getChar()));
    vm->push(v);
}

void
Primitives::streamToken(VM::Process* proc) {
    // TODO: when strings are ready
}

void
Primitives::ieq(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Value((a.i32 == b.i32) ? -1 : 0));
}

void
Primitives::ineq(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Value((a.i32 != b.i32) ? -1 : 0));
}

void
Primitives::igt(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Value(a.i32 > b.i32));
}

void
Primitives::ilt(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Value(a.i32 < b.i32));
}

void
Primitives::igeq(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Value(a.i32 >= b.i32));
}

void
Primitives::ileq(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Value(a.i32 <= b.i32));
}

void
Primitives::notBW(VM::Process* proc) {
    VS_POP(v);
    proc->pushValue(VM::Value(!v.u32));
}

void
Primitives::andBW(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Value(a.u32 & b.u32));
}

void
Primitives::orBW(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Value(a.u32 | b.u32));
}

void
Primitives::vsPtr(VM::Process* proc) {
    VM::Value v(static_cast<int32_t>(proc->valueStack_.size()) - 1);
    proc->pushValue(v);
}

void
Primitives::rsPtr(VM::Process* proc) {
    VM::Value v(static_cast<int32_t>(proc->returnStack_.size()) - 1);
    proc->pushValue(v);
}

void
Primitives::wsPtr(VM::Process* proc) {
    VM::Value v(static_cast<int32_t>(proc->vm_->wordSegment.size()) - 1);
    proc->pushValue(v);
}

void
Primitives::cdsPtr(VM::Process* proc) {
    VM::Value v(static_cast<int32_t>(proc->vm_->constDataSegment.size()) - 1);
    proc->pushValue(v);
}

void
Primitives::vsFetch(VM::Process* proc) {
    VS_POP(addr);
    VM::Value v = proc->valueStack_[addr.i32];
    proc->pushValue(v);
}

void
Primitives::rsFetch(VM::Process* proc) {
    VS_POP(addr);
    VM::Value v(static_cast<int32_t>(proc->returnStack_[addr.i32].ip));
    proc->pushValue(v);
}

void
Primitives::lsFetch(VM::Process* proc) {
    VS_POP(addr);

    uint32_t lp     = proc->lp_ + addr.u32;

    VM::Value v = vm->localStack[lp];
    vm->push(v);
}

void
Primitives::wsFetch(VM::Process* proc) {
    VS_POP(addr);

    VM::Value v(static_cast<int32_t>(vm->wordSegment[addr.i32]));
    vm->push(v);
}

void
Primitives::cdsFetch(VM::Process* proc) {
    VS_POP(addr);
    VM::Value v = vm->constDataSegment[addr.i32];
    vm->push(v);
}

void
Primitives::esFetch(VM::Process* proc) {
    VS_POP(addr);
    VM::ErrorCase v = vm->exceptionStack[addr.i32].errorCase;
    vm->push(VM::Value(static_cast<int32_t>(v)));
}

void
Primitives::vsStore(VM::Process* proc) {
    VS_POP(addr);
    VS_POP(v);

    vm->valueStack[addr.i32] = v;
}

void
Primitives::lsStore(VM::Process* proc) {
    VS_POP(addr);
    VS_POP(v);

    uint32_t lp     = vm->lp + addr.u32;

    vm->localStack[lp] = v;
}

void
Primitives::wsStore(VM::Process* proc) {
    VS_POP(addr);
    VS_POP(v);

    vm->wordSegment[addr.i32] = v.u32;
}

void
Primitives::cdsStore(VM::Process* proc) {
    VS_POP(addr);
    VS_POP(v);

    vm->constDataSegment[addr.i32] = v;
}

void
Primitives::esStore(VM::Process* proc) {
    VS_POP(addr);
    VS_POP(v);

    vm->exceptionStack[addr.i32] = VM::Error(static_cast<VM::ErrorCase>(v.i32), "");
}

void
Primitives::bye(VM::Process* proc) {
    vm->sig = VM::Signal::ABORT;
}

void
Primitives::exit(VM::Process* proc) {
    VS_POP(ret);
    ::exit(ret.i32);
}

void
Primitives::showValueStack(VM::Process* proc) {
    for( size_t i = 0; i < vm->valueStack.size(); ++i ) {
        fprintf(stdout, "vs@%d -- 0x%X\n", i, vm->valueStack[i].u32);
    }
}

void
Primitives::see(VM::Process* proc) {
    uint32_t    word    = vm->nameToWord[vm->getToken()];
    fprintf(stdout, "[%d] : %s ", word, vm->functions[word].name.c_str());
    if( vm->functions[word].native ) {
         fprintf(stdout, " <native> ");
    } else {
        int32_t     curr    = vm->functions[word].start;
        while( vm->wordSegment[curr] != 1 ) {
            if( vm->wordSegment[curr] == 0 ) {
                fprintf(stdout, "%d ", vm->wordSegment[++curr]);
            } else {
                fprintf(stdout, "@%d:%s ", curr, vm->functions[vm->wordSegment[curr]].name.c_str());
            }

            ++curr;
        }
    }

    if( vm->functions[word].isImmediate ) {
        fprintf(stdout, "immediate");
    }

    fprintf(stdout, "\n");
}

void
Primitives::setDebugMode(VM::Process* proc) {
    VS_POP(v);
    vm->verboseDebugging    = v.u32 ? true : false;
}

}   // namespace forth
