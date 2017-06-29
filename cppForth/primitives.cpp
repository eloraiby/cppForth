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

    vm->setBranch(addr.i32);

    return VM::State::NO_ERROR;
}

VM::State
Primitives::branchIf(VM* vm) {
    VM::Value cond  = vm->top();
    vm->pop();

    VM::Value addr  = vm->top();
    vm->pop();

    if( cond.i32 != 0 ) {
        vm->setBranch(addr.i32);
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
Primitives::emitWord(VM* vm) {
    VM::Value v   = vm->top();
    vm->pop();

    if( v.i32 < 0 || v.u32 >= vm->functions.size() ) {
        return VM::State::WORD_NOT_FOUND;
    } else {
        vm->emit(v.u32);
        return VM::State::NO_ERROR;
    }
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
    vm->push(VM::Value(a.i32 == b.i32));
    return VM::State::NO_ERROR;
}

VM::State
Primitives::ineq(VM* vm) {
    VM::Value b   = vm->top();
    vm->pop();
    VM::Value a   = vm->top();
    vm->pop();
    vm->push(VM::Value(a.i32 != b.i32));
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

}   // namespace forth
