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

VM::State
Primitives::fetchInt32(VM* vm) {
    int32_t    u   = vm->fetch();
    VM::Value   v(u);
    vm->push(v);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::returnWord(VM* vm) {
    vm->setRet();
    return VM::State::NO_ERROR;
}

VM::State
Primitives::wordId(VM* vm) {
    std::string name    = vm->getToken();
    
    if( vm->isInt(name) ) {
        return VM::State::INT_IS_NO_WORD;
    }

    if( vm->nameToWord.find(name) == vm->nameToWord.end() ) {
        std::cerr << "ERROR: word not found (" << name << ")" << std::endl;
        return VM::State::WORD_NOT_FOUND;
    }
    
    uint32_t    wordId = vm->nameToWord[name];
    vm->emit(0);
    vm->emit(wordId);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::printInt32(VM* vm) {
    VM::Value   v   = vm->top();
    vm->pop();
    std::cout << v.i32 << std::endl;
    return VM::State::NO_ERROR;
}

VM::State
Primitives::printChar(VM* vm) {
    VM::Value   v   = vm->top();
    vm->pop();
    std::cout << static_cast<char>(v.i32);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::defineWord(VM* vm) {

    std::string name    = vm->getToken();

    if( VM::isInt(name) ) {
        return VM::State::INT_IS_NO_WORD;
    } else {
        //
        // TODO:    do we want to allow forward declaration ?
        //          In this case, we should test to see if the functions[nameToWord[name]].start == -1 && .native == nullptr
        //          before setting the the old function to a value
        //
        uint32_t    wordId  = static_cast<uint32_t>(vm->functions.size());

        VM::Function    func;
#ifdef _DEBUG
        func.name   = name;
#endif
        func.start  = vm->words.size();
        vm->functions.push_back(func);

        vm->nameToWord[name]    = wordId;
        
        vm->stream()->setMode(IStream::Mode::COMPILE);

        return VM::State::NO_ERROR;
    }
}

VM::State
Primitives::immediate(VM* vm) {
    vm->functions[vm->functions.size() - 1].isImmediate = true;
    return VM::State::NO_ERROR;
}

VM::State
Primitives::addInt32(VM* vm) {
    VM::Value b   = vm->top();
    vm->pop();
    VM::Value a   = vm->top();
    vm->pop();
    vm->push(VM::Value(a.i32 + b.i32));
    return VM::State::NO_ERROR;
}

VM::State
Primitives::subInt32(VM* vm) {
    VM::Value b   = vm->top();
    vm->pop();
    VM::Value a   = vm->top();
    vm->pop();
    vm->push(VM::Value(a.i32 - b.i32));
    return VM::State::NO_ERROR;
}

VM::State
Primitives::mulInt32(VM* vm) {
    VM::Value b   = vm->top();
    vm->pop();
    VM::Value a   = vm->top();
    vm->pop();
    vm->push(VM::Value(a.i32 * b.i32));
    return VM::State::NO_ERROR;
}

VM::State
Primitives::divInt32(VM* vm) {
    VM::Value b   = vm->top();
    vm->pop();
    VM::Value a   = vm->top();
    vm->pop();
    vm->push(VM::Value(a.i32 / b.i32));
    return VM::State::NO_ERROR;
}

VM::State
Primitives::modInt32(VM* vm) {
    VM::Value b   = vm->top();
    vm->pop();
    VM::Value a   = vm->top();
    vm->pop();
    vm->push(VM::Value(a.i32 % b.i32));
    return VM::State::NO_ERROR;
}

VM::State
Primitives::branch(VM* vm) {

    VM::Value addr  = vm->top();
    vm->pop();

    vm->setBranch(addr.i32 - 1);

    return VM::State::NO_ERROR;
}

VM::State
Primitives::branchIf(VM* vm) {
    VM::Value addr  = vm->top();
    vm->pop();

    VM::Value cond  = vm->top();
    vm->pop();

    if( cond.i32 != 0 ) {
        vm->setBranch(addr.i32 - 1);
    }

    return VM::State::NO_ERROR;
}

VM::State
Primitives::dup(VM* vm) {
    VM::Value val  = vm->top();
    vm->push(val);

    return VM::State::NO_ERROR;
}

VM::State
Primitives::drop(VM* vm) {
    vm->pop();

    return VM::State::NO_ERROR;
}

VM::State
Primitives::swap(VM* vm) {
    VM::Value v0  = vm->top();
    vm->pop();

    VM::Value v1  = vm->top();
    vm->pop();

    vm->push(v0);
    vm->push(v1);

    return VM::State::NO_ERROR;
}

VM::State
Primitives::codeSize(VM* vm) {
    VM::Value v(static_cast<int32_t>(vm->words.size()));
    vm->push(v);

    return VM::State::NO_ERROR;
}

VM::State
Primitives::endWord(VM* vm) {
    vm->stream()->setMode(IStream::Mode::EVAL);
    vm->emit(1);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::emitReturn(VM* vm) {
    VM::Value v   = vm->top();
    vm->pop();

    vm->returnStack.push_back(v.u32);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::emitWord(VM* vm) {
    VM::Value v   = vm->top();
    vm->pop();
    vm->emit(v.u32);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::emitConstData(VM* vm) {
    VM::Value v = vm->top();
    vm->pop();

    vm->constDataStack.push_back(v);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::emitException(VM* vm) {
    VM::Value v = vm->top();
    vm->pop();

    vm->exceptionStack.push_back(static_cast<VM::State>(v.i32));
    return VM::State::NO_ERROR;
}

VM::State
Primitives::streamPeek(VM* vm) {
    // TODO: handle stream error (does not exist)
    VM::Value v(static_cast<uint32_t>(vm->stream()->peekChar()));
    vm->push(v);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::streamGetCH(VM* vm) {
    // TODO: handle stream error (does not exist)
    VM::Value v(static_cast<uint32_t>(vm->stream()->getChar()));
    vm->push(v);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::streamToken(VM* vm) {
    // TODO: when strings are ready
    return VM::State::NO_ERROR;
}

VM::State
Primitives::ieq(VM* vm) {
    VM::Value b   = vm->top();
    vm->pop();
    VM::Value a   = vm->top();
    vm->pop();
    vm->push(VM::Value((a.i32 == b.i32) ? -1 : 0));
    return VM::State::NO_ERROR;
}

VM::State
Primitives::ineq(VM* vm) {
    VM::Value b   = vm->top();
    vm->pop();
    VM::Value a   = vm->top();
    vm->pop();
    vm->push(VM::Value((a.i32 != b.i32) ? -1 : 0));
    return VM::State::NO_ERROR;
}

VM::State
Primitives::igt(VM* vm) {
    VM::Value b   = vm->top();
    vm->pop();
    VM::Value a   = vm->top();
    vm->pop();
    vm->push(VM::Value(a.i32 > b.i32));
    return VM::State::NO_ERROR;
}

VM::State
Primitives::ilt(VM* vm) {
    VM::Value b   = vm->top();
    vm->pop();
    VM::Value a   = vm->top();
    vm->pop();
    vm->push(VM::Value(a.i32 < b.i32));
    return VM::State::NO_ERROR;
}

VM::State
Primitives::igeq(VM* vm) {
    VM::Value b   = vm->top();
    vm->pop();
    VM::Value a   = vm->top();
    vm->pop();
    vm->push(VM::Value(a.i32 >= b.i32));
    return VM::State::NO_ERROR;
}

VM::State
Primitives::ileq(VM* vm) {
    VM::Value b   = vm->top();
    vm->pop();
    VM::Value a   = vm->top();
    vm->pop();
    vm->push(VM::Value(a.i32 <= b.i32));
    return VM::State::NO_ERROR;
}

VM::State
Primitives::notBW(VM* vm) {
    VM::Value v   = vm->top();
    vm->pop();

    vm->push(VM::Value(!v.u32));
    return VM::State::NO_ERROR;
}

VM::State
Primitives::andBW(VM* vm) {
    VM::Value b   = vm->top();
    vm->pop();
    VM::Value a   = vm->top();
    vm->pop();
    vm->push(VM::Value(a.u32 & b.u32));
    return VM::State::NO_ERROR;
}

VM::State
Primitives::orBW(VM* vm) {
    VM::Value b   = vm->top();
    vm->pop();
    VM::Value a   = vm->top();
    vm->pop();
    vm->push(VM::Value(a.u32 | b.u32));
    return VM::State::NO_ERROR;
}

VM::State
Primitives::vsPtr(VM *vm) {
    VM::Value v(static_cast<int32_t>(vm->valueStack.size()) - 1);
    vm->push(v);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::rsPtr(VM *vm) {
    VM::Value v(static_cast<int32_t>(vm->returnStack.size()) - 1);
    vm->push(v);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::wsPtr(VM *vm) {
    VM::Value v(static_cast<int32_t>(vm->words.size()) - 1);
    vm->push(v);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::cdsPtr(VM* vm) {
    VM::Value v(static_cast<int32_t>(vm->constDataStack.size()) - 1);
    vm->push(v);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::esPtr(VM* vm) {
    VM::Value v(static_cast<int32_t>(vm->exceptionStack.size()) - 1);
    vm->push(v);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::vsFetch(VM *vm) {
    VM::Value addr   = vm->top();
    vm->pop();

    VM::Value v = vm->valueStack[addr.i32];
    vm->push(v);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::rsFetch(VM *vm) {
    VM::Value addr   = vm->top();
    vm->pop();

    VM::Value v(static_cast<int32_t>(vm->returnStack[addr.i32]));
    vm->push(v);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::wsFetch(VM *vm) {
    VM::Value addr   = vm->top();
    vm->pop();

    VM::Value v(static_cast<int32_t>(vm->words[addr.i32]));
    vm->push(v);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::cdsFetch(VM *vm) {
    VM::Value addr   = vm->top();
    vm->pop();

    VM::Value v = vm->constDataStack[addr.i32];
    vm->push(v);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::esFetch(VM *vm) {
    VM::Value addr   = vm->top();
    vm->pop();

    VM::State v = vm->exceptionStack[addr.i32];
    vm->push(VM::Value(static_cast<int32_t>(v)));
    return VM::State::NO_ERROR;
}

VM::State
Primitives::vsStore(VM *vm) {
    VM::Value addr  = vm->top();
    vm->pop();

    VM::Value v     = vm->top();
    vm->pop();

    vm->valueStack[addr.i32] = v;
    return VM::State::NO_ERROR;
}

VM::State
Primitives::rsStore(VM *vm) {
    VM::Value addr  = vm->top();
    vm->pop();

    VM::Value v     = vm->top();
    vm->pop();

    vm->returnStack[addr.i32] = v.u32;
    return VM::State::NO_ERROR;
}

VM::State
Primitives::wsStore(VM *vm) {
    VM::Value addr  = vm->top();
    vm->pop();

    VM::Value v     = vm->top();
    vm->pop();

    vm->words[addr.i32] = v.u32;
    return VM::State::NO_ERROR;
}

VM::State
Primitives::cdsStore(VM *vm) {
    VM::Value addr  = vm->top();
    vm->pop();

    VM::Value v     = vm->top();
    vm->pop();

    vm->constDataStack[addr.i32] = v;
    return VM::State::NO_ERROR;
}

VM::State
Primitives::esStore(VM *vm) {
    VM::Value addr  = vm->top();
    vm->pop();

    VM::Value v     = vm->top();
    vm->pop();

    vm->exceptionStack[addr.i32] = static_cast<VM::State>(v.i32);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::exit(VM *vm) {
    VM::Value ret    = vm->top();
    vm->pop();
    ::exit(ret.i32);
    return VM::State::NO_ERROR;
}

VM::State
Primitives::showValueStack(VM* vm) {
    for( size_t i = 0; i < vm->valueStack.size(); ++i ) {
        std::cout << "vs@" << i << " -- " << std::hex << vm->valueStack[i].u32 << std::dec << std::endl;
    }
    return VM::State::NO_ERROR;
}

VM::State
Primitives::see(VM *vm) {
    uint32_t    word    = vm->nameToWord[vm->getToken()];
    std::cout << "[" << word << "] : " << vm->functions[word].name << " ";
    if( vm->functions[word].native ) {
         std::cout << " <native> ";
    } else {
        int32_t     curr    = vm->functions[word].start;
        while( vm->words[curr] != 1 ) {
            if( vm->words[curr] == 0 ) {
                std::cout << vm->words[++curr] << " ";
            } else {
                std::cout << "@" << curr << ":" << vm->functions[vm->words[curr]].name << " ";
            }

            ++curr;
        }
    }

    if( vm->functions[word].isImmediate ) {
        std::cout << "immediate";
    }

    std::cout << std::endl;

    return VM::State::NO_ERROR;
}

}   // namespace forth
