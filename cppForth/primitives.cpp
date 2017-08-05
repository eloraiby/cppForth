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
    if( vm->valueStack.size() == 0 ) { vm->throwException(VM::ErrorCase::VS_UNDERFLOW, "value stack underflow"); return; } \

#define VS_POP(V)   \
    if( vm->valueStack.size() == 0 ) { vm->throwException(VM::ErrorCase::VS_UNDERFLOW, "value stack underflow"); return; } \
    VM::Value   V = vm->top(); \
    vm->pop()

void
Primitives::fetchInt32(VM* vm) {
    int32_t    u   = vm->fetch();
    VM::Value   v(u);
    vm->push(v);
}

void
Primitives::returnWord(VM* vm) {
    vm->setRet();
}

void
Primitives::wordId(VM* vm) {
    String name    = vm->getToken();
    
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
Primitives::callIndirect(VM* vm) {
    VS_POP(u);
    vm->setCall(u.u32);
    --vm->wp;   // once outside the native, wp will get incremented, so decrement to stay at the start of the word
}

void
Primitives::printInt32(VM* vm) {
    VS_POP(v);
    fprintf(stdout, "%d\n", v.i32);
}

void
Primitives::printChar(VM* vm) {
    VS_POP(v);
    fprintf(stdout, "%c", static_cast<char>(v.i32));
}

void
Primitives::defineWord(VM* vm) {

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
Primitives::immediate(VM* vm) {
    vm->functions[vm->functions.size() - 1].isImmediate = true;
}

void
Primitives::setLocalCount(VM* vm) {
    String tok  = vm->getToken();

    if( !VM::isInt(tok) ) {
        vm->throwException(VM::ErrorCase::LOCAL_IS_NOT_INT, "a local should be an integer");
        return;
    }

    vm->functions[vm->functions.size() - 1].localCount = VM::toInt32(tok);
}

void
Primitives::addInt32(VM* vm) {
    VS_POP(b);
    VS_POP(a);
    vm->push(VM::Value(a.i32 + b.i32));
}

void
Primitives::subInt32(VM* vm) {
    VS_POP(b);
    VS_POP(a);
    vm->push(VM::Value(a.i32 - b.i32));
}

void
Primitives::mulInt32(VM* vm) {
    VS_POP(b);
    VS_POP(a);
    vm->push(VM::Value(a.i32 * b.i32));
}

void
Primitives::divInt32(VM* vm) {
    VS_POP(b);
    VS_POP(a);
    vm->push(VM::Value(a.i32 / b.i32));
}

void
Primitives::modInt32(VM* vm) {
    VS_POP(b);
    VS_POP(a);
    vm->push(VM::Value(a.i32 % b.i32));
}

void
Primitives::branch(VM* vm) {
    VS_POP(addr);
    vm->setBranch(addr.i32 - 1);
}

void
Primitives::branchIf(VM* vm) {
    VS_POP(addr);
    VS_POP(cond);

    if( cond.i32 != 0 ) {
        vm->setBranch(addr.i32 - 1);
    }
}

void
Primitives::dup(VM* vm) {
    VS_CHECK()
    VM::Value val   = vm->top();
    vm->push(val);
}

void
Primitives::drop(VM* vm) {
    VS_CHECK()
    vm->pop();
}

void
Primitives::swap(VM* vm) {
    VS_POP(v0);
    VS_POP(v1);

    vm->push(v0);
    vm->push(v1);
}

void
Primitives::codeSize(VM* vm) {
    VM::Value v(static_cast<int32_t>(vm->wordSegment.size()));
    vm->push(v);
}

void
Primitives::endWord(VM* vm) {
    vm->stream()->setMode(IInputStream::Mode::EVAL);
    vm->emit(1);
}

void
Primitives::emitWord(VM* vm) {
    VS_POP(v);
    vm->emit(v.u32);
}

void
Primitives::emitConstData(VM* vm) {
    VS_POP(v);
    vm->constDataSegment.push_back(v);
}

void
Primitives::emitException(VM* vm) {
    VS_POP(v);
    vm->exceptionStack.push_back(VM::Error(static_cast<VM::ErrorCase>(v.i32), ""));
}

void
Primitives::streamPeek(VM* vm) {
    // TODO: handle stream error (does not exist)
    VM::Value v(static_cast<uint32_t>(vm->stream()->peekChar()));
    vm->push(v);
}

void
Primitives::streamGetCH(VM* vm) {
    // TODO: handle stream error (does not exist)
    VM::Value v(static_cast<uint32_t>(vm->stream()->getChar()));
    vm->push(v);
}

void
Primitives::streamToken(VM* vm) {
    // TODO: when strings are ready
}

void
Primitives::ieq(VM* vm) {
    VS_POP(b);
    VS_POP(a);
    vm->push(VM::Value((a.i32 == b.i32) ? -1 : 0));
}

void
Primitives::ineq(VM* vm) {
    VS_POP(b);
    VS_POP(a);
    vm->push(VM::Value((a.i32 != b.i32) ? -1 : 0));
}

void
Primitives::igt(VM* vm) {
    VS_POP(b);
    VS_POP(a);
    vm->push(VM::Value(a.i32 > b.i32));
}

void
Primitives::ilt(VM* vm) {
    VS_POP(b);
    VS_POP(a);
    vm->push(VM::Value(a.i32 < b.i32));
}

void
Primitives::igeq(VM* vm) {
    VS_POP(b);
    VS_POP(a);
    vm->push(VM::Value(a.i32 >= b.i32));
}

void
Primitives::ileq(VM* vm) {
    VS_POP(b);
    VS_POP(a);
    vm->push(VM::Value(a.i32 <= b.i32));
}

void
Primitives::notBW(VM* vm) {
    VS_POP(v);
    vm->push(VM::Value(!v.u32));
}

void
Primitives::andBW(VM* vm) {
    VS_POP(b);
    VS_POP(a);
    vm->push(VM::Value(a.u32 & b.u32));
}

void
Primitives::orBW(VM* vm) {
    VS_POP(b);
    VS_POP(a);
    vm->push(VM::Value(a.u32 | b.u32));
}

void
Primitives::vsPtr(VM *vm) {
    VM::Value v(static_cast<int32_t>(vm->valueStack.size()) - 1);
    vm->push(v);
}

void
Primitives::rsPtr(VM *vm) {
    VM::Value v(static_cast<int32_t>(vm->returnStack.size()) - 1);
    vm->push(v);
}

void
Primitives::wsPtr(VM *vm) {
    VM::Value v(static_cast<int32_t>(vm->wordSegment.size()) - 1);
    vm->push(v);
}

void
Primitives::cdsPtr(VM* vm) {
    VM::Value v(static_cast<int32_t>(vm->constDataSegment.size()) - 1);
    vm->push(v);
}

void
Primitives::esPtr(VM* vm) {
    VM::Value v(static_cast<int32_t>(vm->exceptionStack.size()) - 1);
    vm->push(v);
}

void
Primitives::vsFetch(VM *vm) {
    VS_POP(addr);
    VM::Value v = vm->valueStack[addr.i32];
    vm->push(v);
}

void
Primitives::rsFetch(VM *vm) {
    VS_POP(addr);
    VM::Value v(static_cast<int32_t>(vm->returnStack[addr.i32].ip));
    vm->push(v);
}

void
Primitives::lsFetch(VM* vm) {
    VS_POP(addr);

    uint32_t lp     = vm->lp + addr.u32;

    VM::Value v = vm->localStack[lp];
    vm->push(v);
}

void
Primitives::wsFetch(VM *vm) {
    VS_POP(addr);

    VM::Value v(static_cast<int32_t>(vm->wordSegment[addr.i32]));
    vm->push(v);
}

void
Primitives::cdsFetch(VM *vm) {
    VS_POP(addr);
    VM::Value v = vm->constDataSegment[addr.i32];
    vm->push(v);
}

void
Primitives::esFetch(VM *vm) {
    VS_POP(addr);
    VM::ErrorCase v = vm->exceptionStack[addr.i32].errorCase;
    vm->push(VM::Value(static_cast<int32_t>(v)));
}

void
Primitives::vsStore(VM *vm) {
    VS_POP(addr);
    VS_POP(v);

    vm->valueStack[addr.i32] = v;
}

void
Primitives::lsStore(VM* vm) {
    VS_POP(addr);
    VS_POP(v);

    uint32_t lp     = vm->lp + addr.u32;

    vm->localStack[lp] = v;
}

void
Primitives::wsStore(VM *vm) {
    VS_POP(addr);
    VS_POP(v);

    vm->wordSegment[addr.i32] = v.u32;
}

void
Primitives::cdsStore(VM *vm) {
    VS_POP(addr);
    VS_POP(v);

    vm->constDataSegment[addr.i32] = v;
}

void
Primitives::esStore(VM *vm) {
    VS_POP(addr);
    VS_POP(v);

    vm->exceptionStack[addr.i32] = VM::Error(static_cast<VM::ErrorCase>(v.i32), "");
}

void
Primitives::bye(VM *vm) {
    vm->sig = VM::Signal::ABORT;
}

void
Primitives::exit(VM *vm) {
    VS_POP(ret);
    ::exit(ret.i32);
}

void
Primitives::showValueStack(VM* vm) {
    for( size_t i = 0; i < vm->valueStack.size(); ++i ) {
        fprintf(stdout, "vs@%d -- 0x%X\n", i, vm->valueStack[i].u32);
    }
}

void
Primitives::see(VM *vm) {
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
Primitives::setDebugMode(VM* vm) {
    VS_POP(v);
    vm->verboseDebugging    = v.u32 ? true : false;
}

}   // namespace forth
