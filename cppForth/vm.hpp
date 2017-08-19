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
#ifndef __VM__HPP__
#define __VM__HPP__
#ifndef __SM_BASE__
#   include "base.hpp"
#endif

#include "intrusive-ptr.hpp"
#include "vector.hpp"
#include "string.hpp"
#include "hash_map.hpp"

namespace SM {
struct VM : public RCObject {

    struct Process;

    typedef void    (*NativeFunction)(Process* proc);

    struct Function {
        enum Color {
            NATIVE                      = 0,    // native function
            NORMAL                      = 1,    // normal interpreted
        };

        String              name;           // keep this even in release for debugging purpose
        Color               color;
        bool                isImmediate;    // this is only needed in the parsing phase, it will simplify the interpreter later

        union {
            NativeFunction      native;
            struct {
                int32_t             start;
                uint32_t            localCount;
            } interpreted;
        } body;

        inline bool             isNative() const { return color == NATIVE; }

        Function() : color(NATIVE), isImmediate(false) {
            body.native = nullptr;
            body.interpreted.localCount = 0;
        }
    };

    struct Process : public RCObject {
        typedef IntrusivePtr<Process>   Ptr;

        struct Signal {
            enum Type {
                NONE                    =  0,   // no sginal, all normal
                EXIT                    = -1,   // exit normally
                EXCEPTION               = -2,   // exception
                WORD_ID_OUT_OF_RANGE    = -3,   // code segment fault
                WORD_NOT_IMPLEMENTED    = -4,   // the function is not implemented (TODO: should this be on the parser end only ?)
                VS_UNDERFLOW            = -5,   // value stack underflow

            };

            Type                ty;     // signal type
            uint32_t            pid;    // originating process id
            uint32_t            data;   // signal data

            Signal(Type ty, uint32_t pid, uint32_t data) : ty(ty), pid(pid), data(data) {}
        };

        ///
        /// return stack entry
        ///
        struct RetEntry {
            uint32_t            word;   // calling word
            uint32_t            ip; // global text (code) instruction pointer
            uint32_t            lp; // local pointer
            uint32_t            cp; // exception catcher (catch pointer)
        };

        union Value {
            uint32_t            u32;
            int32_t             i32;
            float               f32;
            void*               ptr;
        
            Value()                     : u32(0) {}
            explicit Value(uint32_t v)  : u32(v) {}
            explicit Value(int32_t v)   : i32(v) {}
            explicit Value(float v)     : f32(v) {}
            explicit Value(void* v)     : ptr(v) {}
        };

        inline void     pushValue(Value v)      { valueStack_.push_back(v); }
        inline Value    topValue() const        { return valueStack_.back(); }
        inline void     popValue()              { valueStack_.pop_back(); }

        void            step();
        void            runCall(uint32_t word);
        void            emitSignal(const Signal& sig);
    
        Process(Process* parent, uint32_t pid);

        uint32_t        pid() const             { return pid_; }


    protected:
        inline void
        setCall(uint32_t word) {
            RetEntry re;
            re.word = word;
            re.ip = wp_;
            re.lp = lp_;
            returnStack_.push_back(re);
            wp_ = vm_->functions_[word].body.interpreted.start;
            lp_ = localStack_.size();
            localStack_.resize(lp_ + vm_->functions_[word].body.interpreted.localCount);
        }

        inline void
        setRet() {
            uint32_t word = returnStack_.back().word;
            wp_ = returnStack_.back().ip;
            lp_  = returnStack_.back().lp;
            localStack_.resize(localStack_.size() - vm_->functions_[word].body.interpreted.localCount);
            returnStack_.pop_back();
        }

        inline void     setBranch(uint32_t addr)    { wp_ = addr; }

        uint32_t        fetch()                     { ++wp_; return vm_->wordSegment_[wp_]; } 

        uint32_t                                pid_;           // process id

        uint32_t                                wp_;            // instruction pointer
        uint32_t                                lp_;            // local pointer
        Signal                                  sig_;           // high priority interrupt

        VM*                                     vm_;            // the virtual machine this process belongs to
        Process*                                parent_;        // parent process

        Vector<Value>                           valueStack_;    // contains values on the stack
        Vector<RetEntry>                        returnStack_;   // contains calling word pointer
        Vector<Value>                           localStack_;    // local block stack

        friend struct Primitives;
    };

    int32_t         findWord(const String& name);


    inline uint32_t wordAddr(uint32_t word)     { return functions_[word].body.interpreted.start; }

    inline uint32_t emit(uint32_t word)         { uint32_t pos = static_cast<uint32_t>(wordSegment_.size()); wordSegment_.push_back(word); return pos; }

    uint32_t        addNativeFunction(const String& name, NativeFunction native, bool isImmediate);
    uint32_t        addNormalFunction(const String& name);

    void            setFunctionAsImmediate(uint32_t idx) { functions_[idx].isImmediate = true; }
    void            setFunctionLocalCount(uint32_t idx, uint32_t locals) { functions_[idx].body.interpreted.localCount = locals; }


    VM();
    inline const Vector<Function>&              functions() const { return functions_; }
    inline const HashMap<String, uint32_t>&     nameToWord() const { return nameToWord_; }

    const Vector<uint32_t>& wordSegment() const { return wordSegment_; }
    inline uint32_t wordSegmentSize() const     { return wordSegment_.size(); }
    inline bool     isVerboseDebugging() const  { return verboseDebugging; }

private:

    void            initPrimitives();

    Vector<Function>                            functions_;
    HashMap<String, uint32_t>                   nameToWord_;

    Vector<uint32_t>                            wordSegment_;    // the code segment
    Vector<Process::Value>                      constDataSegment;   // strings, names, ...


    // debugging facilites
    bool                                        verboseDebugging;


    friend struct   Primitives;
};

struct Primitives {
    // primitives
    static void     fetchInt32      (VM::Process* proc);
    static void     returnWord      (VM::Process* proc);
    static void     callIndirect    (VM::Process* proc);

    static void     printInt32      (VM::Process* proc);
    static void     printChar       (VM::Process* proc);
    static void     addInt32        (VM::Process* proc);
    static void     subInt32        (VM::Process* proc);
    static void     mulInt32        (VM::Process* proc);
    static void     divInt32        (VM::Process* proc);
    static void     modInt32        (VM::Process* proc);
    static void     branch          (VM::Process* proc);
    static void     branchIf        (VM::Process* proc);
    static void     dup             (VM::Process* proc);
    static void     drop            (VM::Process* proc);
    static void     swap            (VM::Process* proc);
    static void     codeSize        (VM::Process* proc);

    static void     emitWord        (VM::Process* proc);
    static void     emitConstData   (VM::Process* proc);
    static void     emitException   (VM::Process* proc);

    static void     ieq             (VM::Process* proc);
    static void     ineq            (VM::Process* proc);
    static void     igt             (VM::Process* proc);
    static void     ilt             (VM::Process* proc);
    static void     igeq            (VM::Process* proc);
    static void     ileq            (VM::Process* proc);
    static void     notBW           (VM::Process* proc);
    static void     andBW           (VM::Process* proc);
    static void     orBW            (VM::Process* proc);

    // machine stacks
    static void     vsPtr           (VM::Process* proc);
    static void     rsPtr           (VM::Process* proc);
    static void     wsPtr           (VM::Process* proc);
    static void     cdsPtr          (VM::Process* proc);

    static void     vsFetch         (VM::Process* proc);
    static void     rsFetch         (VM::Process* proc);
    static void     lsFetch         (VM::Process* proc);
    static void     wsFetch         (VM::Process* proc);
    static void     cdsFetch        (VM::Process* proc);

    static void     vsStore         (VM::Process* proc);
    static void     lsStore         (VM::Process* proc);
    static void     wsStore         (VM::Process* proc);
    static void     cdsStore        (VM::Process* proc);

    static void     bye             (VM::Process* proc);
    static void     exit            (VM::Process* proc);

    static void     pid             (VM::Process* proc);

    // debug helpers
    static void     showValueStack  (VM::Process* proc);
    static void     setDebugMode    (VM::Process* proc);
};
} // namespace SM
#endif