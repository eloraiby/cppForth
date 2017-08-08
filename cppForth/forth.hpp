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
#ifndef __FORTH__HPP__
#define __FORTH__HPP__

#ifndef __FORTH_BASE__
#   include "base.hpp"
#endif

#include "intrusive-ptr.hpp"
#include "vector.hpp"
#include "string.hpp"
#include "hash_map.hpp"

namespace Forth {

enum { MAX_BUFF = 1024 };

struct IInputStream : public RCObject {
    typedef IntrusivePtr<IInputStream>  Ptr;
    IInputStream() {}

    enum class Mode {
        COMPILE,
        EVAL
    };

    virtual uint32_t        peekChar()      = 0;
    virtual uint32_t        getChar()       = 0;
    virtual Mode            getMode() const = 0;
    virtual void            setMode(Mode m) = 0;
    virtual                 ~IInputStream() = 0;
    
    static inline
    bool
    isSpace(uint32_t ch) {
        return (ch == static_cast<uint32_t>('\n')
            || ch == static_cast<uint32_t>('\r')  // windows
            || ch == static_cast<uint32_t>('\t')
            || ch == static_cast<uint32_t>(' ')
            || ch == static_cast<uint32_t>('\a'));
    }
};

struct VM : public RCObject {

    struct Process;

    enum class ErrorCase {
        WORD_NOT_FOUND          = -1,
        VS_UNDERFLOW            = -2,
        VS_OVERFLOW             = -3,
        RS_UNDERFLOW            = -4,
        RS_OVERFLOW             = -5,
        WORD_NOT_DEFINED        = -6,
        INT_IS_NO_WORD          = -7,
        WORD_ID_OUT_OF_RANGE    = -8,
        LOCAL_IS_NOT_INT        = -9,
        LOCAL_OVERFLOW          = -10,
    };

    enum class Signal {
        NONE                    =  0,   // continue execution
        KILL                    = -1,   // killed
        ABORT                   = -2,   // abort execution
        BREAK                   = -3,   // breakpoint
    };

    struct Error {
        ErrorCase           errorCase;
        String              errorString;

        Error(ErrorCase ec, const String& str) : errorCase(ec), errorString(str) {}
    };

    ///
    /// return stack entry
    ///
    struct RetEntry {
        uint32_t            word;
        uint32_t            ip;
        uint32_t            lp;
        uint32_t            as;
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

    typedef void    (*NativeFunction)(Process* proc);

    struct Function {
        String              name;           // keep this even in release for debugging purpose

        bool                isImmediate;
        NativeFunction      native;
        int32_t             start;
        uint32_t            localCount;

        Function() : isImmediate(false), native(nullptr), start(-1), localCount(0) {}
    };

    struct Process : public RCObject {
        typedef IntrusivePtr<Process>   Ptr;

        inline void     pushValue(Value v)      { valueStack_.push_back(v); }
        inline Value    topValue() const        { return valueStack_.back(); }
        inline void     popValue()              { valueStack_.pop_back(); }

        void            step();
    
    private:
        inline void
        setCall(uint32_t word) {
            RetEntry re;
            re.word = word;
            re.ip = wp_;
            re.lp = lp_;
            returnStack_.push_back(re);
            wp_ = vm_->functions[word].start;
            lp_ = localStack_.size();
            localStack_.resize(lp_ + vm_->functions[word].localCount);
        }

        inline void
        setRet() {
            uint32_t word = returnStack_.back().word;
            wp_ = returnStack_.back().ip;
            lp_  = returnStack_.back().lp;
            localStack_.resize(localStack_.size() - vm_->functions[word].localCount);
            returnStack_.pop_back();
        }

        inline void     setBranch(uint32_t addr)    { wp_ = addr; }

        uint32_t        fetch()                     { ++wp_; return vm_->wordSegment[wp_]; } 

        uint32_t                                wp_;            // instruction pointer
        uint32_t                                lp_;            // local pointer

        VM*                                     vm_;            // the virtual machine this process belongs to
        Process*                                parent_;        // parent process

        Vector<Value>                           valueStack_;    // contains values on the stack
        Vector<RetEntry>                        returnStack_;   // contains calling word pointer
        Vector<Value>                           localStack_;    // local block stack

        friend struct Primitives;
    };

    int32_t         findWord(const String& name);
    void            loadStream(IInputStream::Ptr stream);


    inline uint32_t wordAddr(uint32_t word)     { return functions[word].start; }


    String          getToken();
    void            runCall(uint32_t word);


    inline uint32_t emit(uint32_t word)         { uint32_t pos = static_cast<uint32_t>(wordSegment.size()); wordSegment.push_back(word); return pos; }

    uint32_t        addNativeFunction(const String& name, NativeFunction native, bool isImmediate);

    void            throwException(ErrorCase err, const String& str);

    VM();

private:

    
    inline IInputStream::Ptr    stream() const  { return streams.back(); }
    inline void     pushStream(IInputStream::Ptr strm)  { streams.push_back(strm); }
    inline void     popStream()                 { streams.pop_back(); }

    void            initPrimitives();

    static bool     isInt(const String& tok);
    static int32_t  toInt32(const String& tok);

    Vector<Function>                            functions;
    HashMap<String, uint32_t>                   nameToWord;

    Vector<uint32_t>                            wordSegment;    // the code segment
    Vector<Value>                               constDataSegment;   // strings, names, ...

    Vector<IInputStream::Ptr>                   streams;

    Signal                                      sig;

    // debugging facilites
    bool                                        verboseDebugging;


    friend struct   Primitives;
};

struct StdInStream : public IInputStream {
    void            testAndFillBuffer();
    uint32_t        peekChar() override;
    uint32_t        getChar() override;
    Mode            getMode() const override;
    void            setMode(Mode m) override;

    ~StdInStream() override;
    StdInStream();

    Mode            mode;
    uint32_t        pos;
    Forth::String   buff;
};

struct StringStream : public IInputStream {
    uint32_t        peekChar() override;
    uint32_t        getChar() override;
    Mode            getMode() const override;
    void            setMode(Mode m) override;

    StringStream(const char* str);
    ~StringStream()    override;


    Mode            mode;
    uint32_t        pos;
    Forth::String   buff;
};

struct Primitives {
    // primitives
    static void     fetchInt32      (VM::Process* vm);
    static void     returnWord      (VM::Process* vm);
    static void     wordId          (VM::Process* vm);
    static void     callIndirect    (VM::Process* vm);

    static void     printInt32      (VM::Process* vm);
    static void     printChar       (VM::Process* vm);
    static void     defineWord      (VM::Process* vm);
    static void     immediate       (VM::Process* vm);
    static void     setLocalCount   (VM::Process* vm);
    static void     addInt32        (VM::Process* vm);
    static void     subInt32        (VM::Process* vm);
    static void     mulInt32        (VM::Process* vm);
    static void     divInt32        (VM::Process* vm);
    static void     modInt32        (VM::Process* vm);
    static void     branch          (VM::Process* vm);
    static void     branchIf        (VM::Process* vm);
    static void     dup             (VM::Process* vm);
    static void     drop            (VM::Process* vm);
    static void     swap            (VM::Process* vm);
    static void     codeSize        (VM::Process* vm);
    static void     endWord         (VM::Process* vm);

    static void     emitWord        (VM::Process* vm);
    static void     emitConstData   (VM::Process* vm);
    static void     emitException   (VM::Process* vm);

    static void     streamPeek      (VM::Process* vm);
    static void     streamGetCH     (VM::Process* vm);
    static void     streamToken     (VM::Process* vm);

    static void     ieq             (VM::Process* vm);
    static void     ineq            (VM::Process* vm);
    static void     igt             (VM::Process* vm);
    static void     ilt             (VM::Process* vm);
    static void     igeq            (VM::Process* vm);
    static void     ileq            (VM::Process* vm);
    static void     notBW           (VM::Process* vm);
    static void     andBW           (VM::Process* vm);
    static void     orBW            (VM::Process* vm);

    // machine stacks
    static void     vsPtr           (VM::Process* vm);
    static void     rsPtr           (VM::Process* vm);
    static void     wsPtr           (VM::Process* vm);
    static void     cdsPtr          (VM::Process* vm);
    static void     esPtr           (VM::Process* vm);

    static void     vsFetch         (VM::Process* vm);
    static void     rsFetch         (VM::Process* vm);
    static void     lsFetch         (VM::Process* vm);
    static void     wsFetch         (VM::Process* vm);
    static void     cdsFetch        (VM::Process* vm);
    static void     esFetch         (VM::Process* vm);

    static void     vsStore         (VM::Process* vm);
    static void     lsStore         (VM::Process* vm);
    static void     wsStore         (VM::Process* vm);
    static void     cdsStore        (VM::Process* vm);
    static void     esStore         (VM::Process* vm);



    static void     bye             (VM::Process* vm);
    static void     exit            (VM::Process* vm);

    // debug helpers
    static void     showValueStack  (VM::Process* vm);
    static void     see             (VM::Process* vm);
    static void     setDebugMode    (VM::Process* vm);
};
}

#endif  // __FORTH__HPP__
