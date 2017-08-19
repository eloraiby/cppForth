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

namespace SM {

#define VS_POP(V)   \
    if( proc->valueStack_.size() == 0 ) { proc->emitSignal(VM::Process::Signal(VM::Process::Signal::VS_UNDERFLOW, proc->pid_, 0)); return; } \
    VM::Process::Value   V = proc->topValue(); \
    proc->popValue()

void
Primitives::fetchInt32(VM::Process* proc) {
    int32_t    u   = proc->fetch();
    VM::Process::Value   v(u);
    proc->pushValue(v);
}

void
Primitives::returnWord(VM::Process* proc) {
    proc->setRet();
}

void
Primitives::callIndirect(VM::Process* proc) {
    VS_POP(u);
    proc->setCall(u.u32);
    --proc->wp_;   // once outside the native, wp will get incremented, so decrement to stay at the start of the word
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
Primitives::addInt32(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Process::Value(a.i32 + b.i32));
}

void
Primitives::subInt32(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Process::Value(a.i32 - b.i32));
}

void
Primitives::mulInt32(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Process::Value(a.i32 * b.i32));
}

void
Primitives::divInt32(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Process::Value(a.i32 / b.i32));
}

void
Primitives::modInt32(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Process::Value(a.i32 % b.i32));
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
    VM::Process::Value val   = proc->topValue();
    proc->pushValue(val);
}

void
Primitives::drop(VM::Process* proc) {
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
    VM::Process::Value v(static_cast<int32_t>(proc->vm_->wordSegment_.size()));
    proc->pushValue(v);
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
    proc->emitSignal(VM::Process::Signal(VM::Process::Signal::EXCEPTION, proc->pid_, v.i32));
}

void
Primitives::ieq(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Process::Value((a.i32 == b.i32) ? -1 : 0));
}

void
Primitives::ineq(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Process::Value((a.i32 != b.i32) ? -1 : 0));
}

void
Primitives::igt(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Process::Value(a.i32 > b.i32));
}

void
Primitives::ilt(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Process::Value(a.i32 < b.i32));
}

void
Primitives::igeq(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Process::Value(a.i32 >= b.i32));
}

void
Primitives::ileq(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Process::Value(a.i32 <= b.i32));
}

void
Primitives::notBW(VM::Process* proc) {
    VS_POP(v);
    proc->pushValue(VM::Process::Value(!v.u32));
}

void
Primitives::andBW(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Process::Value(a.u32 & b.u32));
}

void
Primitives::orBW(VM::Process* proc) {
    VS_POP(b);
    VS_POP(a);
    proc->pushValue(VM::Process::Value(a.u32 | b.u32));
}

void
Primitives::vsPtr(VM::Process* proc) {
    VM::Process::Value v(static_cast<int32_t>(proc->valueStack_.size()) - 1);
    proc->pushValue(v);
}

void
Primitives::rsPtr(VM::Process* proc) {
    VM::Process::Value v(static_cast<int32_t>(proc->returnStack_.size()) - 1);
    proc->pushValue(v);
}

void
Primitives::wsPtr(VM::Process* proc) {
    VM::Process::Value v(static_cast<int32_t>(proc->vm_->wordSegment_.size()) - 1);
    proc->pushValue(v);
}

void
Primitives::cdsPtr(VM::Process* proc) {
    VM::Process::Value v(static_cast<int32_t>(proc->vm_->constDataSegment.size()) - 1);
    proc->pushValue(v);
}

void
Primitives::vsFetch(VM::Process* proc) {
    VS_POP(addr);
    VM::Process::Value v = proc->valueStack_[addr.i32];
    proc->pushValue(v);
}

void
Primitives::rsFetch(VM::Process* proc) {
    VS_POP(addr);
    VM::Process::Value v(static_cast<int32_t>(proc->returnStack_[addr.i32].ip));
    proc->pushValue(v);
}

void
Primitives::lsFetch(VM::Process* proc) {
    VS_POP(addr);

    uint32_t lp     = proc->lp_ + addr.u32;

    VM::Process::Value v = proc->localStack_[lp];
    proc->pushValue(v);
}

void
Primitives::wsFetch(VM::Process* proc) {
    VS_POP(addr);

    VM::Process::Value v(static_cast<int32_t>(proc->vm_->wordSegment_[addr.i32]));
    proc->pushValue(v);
}

void
Primitives::cdsFetch(VM::Process* proc) {
    VS_POP(addr);
    VM::Process::Value v = proc->vm_->constDataSegment[addr.i32];
    proc->pushValue(v);
}

void
Primitives::vsStore(VM::Process* proc) {
    VS_POP(addr);
    VS_POP(v);

    proc->valueStack_[addr.i32] = v;
}

void
Primitives::lsStore(VM::Process* proc) {
    VS_POP(addr);
    VS_POP(v);

    uint32_t lp     = proc->lp_ + addr.u32;

    proc->localStack_[lp] = v;
}

void
Primitives::wsStore(VM::Process* proc) {
    VS_POP(addr);
    VS_POP(v);

    proc->vm_->wordSegment_[addr.i32] = v.u32;
}

void
Primitives::cdsStore(VM::Process* proc) {
    VS_POP(addr);
    VS_POP(v);

    proc->vm_->constDataSegment[addr.i32] = v;
}

void
Primitives::bye(VM::Process* proc) {
    proc->sig_ = VM::Process::Signal(VM::Process::Signal::EXIT, proc->pid_, 0);
}

void
Primitives::exit(VM::Process* proc) {
    VS_POP(ret);
    ::exit(ret.i32);
}

void
Primitives::showValueStack(VM::Process* proc) {
    for( size_t i = 0; i < proc->valueStack_.size(); ++i ) {
        fprintf(stdout, "vs@%d -- 0x%X\n", i, proc->valueStack_[i].u32);
    }
}

void
Primitives::setDebugMode(VM::Process* proc) {
    VS_POP(v);
    proc->vm_->verboseDebugging    = v.u32 ? true : false;
}

}   // namespace forth
