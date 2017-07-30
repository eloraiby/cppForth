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

struct IStream {
    typedef IntrusivePtr<IStream>    Ptr;
    IStream() : count_(0) {}

    enum class Mode {
        COMPILE,
        EVAL
    };

    virtual uint32_t        peekChar()      = 0;
    virtual uint32_t        getChar()       = 0;
    virtual Mode            getMode() const = 0;
    virtual void            setMode(Mode m) = 0;
    virtual                 ~IStream()      = 0;
    
    static inline
    bool
    isSpace(uint32_t ch) {
        return (ch == static_cast<uint32_t>('\n')
            || ch == static_cast<uint32_t>('\r')  // windows
            || ch == static_cast<uint32_t>('\t')
            || ch == static_cast<uint32_t>(' ')
            || ch == static_cast<uint32_t>('\a'));
    }

	inline void		grab() const			{ ++count_;		}
	inline void		release() const			{ --count_; if( count_ == 0 ) { delete this; } }
	inline size_t		getRefCount() const		{ return count_;	}

private:
    mutable uint32_t        count_;
};

struct VM {
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
        NONE                    =  0,
        KILL                    = -1,
        ABORT                   = -2,
    };

    struct Error {
        ErrorCase               errorCase;
        String                  errorString;

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

    typedef void    (*NativeFunction)(VM* vm);

    struct Function {
        String              name;           // keep this even in release for debugging purpose

        bool                isImmediate;
        NativeFunction      native;
        int32_t             start;
        uint32_t            localCount;

        Function() : isImmediate(false), native(nullptr), start(-1), localCount(0) {}
    };

    int32_t         findWord(const String& name);
    void            loadStream(IStream::Ptr stream);


    inline uint32_t wordAddr(uint32_t word)     { return functions[word].start; }

    inline void     push(Value v)               { valueStack.push_back(v); }
    inline Value    top() const                 { return valueStack.back(); }
    inline void     pop()                       { valueStack.pop_back(); }

    String          getToken();
    void            step();
    void            runCall(uint32_t word);


    inline uint32_t emit(uint32_t word)         { uint32_t pos = static_cast<uint32_t>(wordSegment.size()); wordSegment.push_back(word); return pos; }

    uint32_t        addNativeFunction(const String& name, NativeFunction native, bool isImmediate);

    void            throwException(ErrorCase err, const String& str);

    VM();

private:
    inline void
    setCall(uint32_t word) {
        RetEntry re;
        re.word = word;
        re.ip = wp;
        re.lp = lp;
        returnStack.push_back(re);
        wp  = functions[word].start;
        lp  = localStack.size();
        localStack.resize(lp + functions[word].localCount);
    }

    inline void
    setRet() {
        uint32_t word = returnStack.back().word;
        wp = returnStack.back().ip;
        lp  = returnStack.back().lp;
        localStack.resize(localStack.size() - functions[word].localCount);
        returnStack.pop_back();
    }

    inline void     setBranch(uint32_t addr)    { wp = addr; }

    uint32_t        fetch()                     { ++wp; return wordSegment[wp]; }
    
    inline IStream::Ptr stream() const          { return streams.back(); }
    inline void     pushStream(IStream::Ptr strm)   { streams.push_back(strm); }
    inline void     popStream()                 { streams.pop_back(); }

    void            initPrimitives();

    static bool     isInt(const String& tok);
    static int32_t  toInt32(const String& tok);

    Vector<Function>                            functions;
    HashMap<String, uint32_t>                   nameToWord;

    Vector<uint32_t>                            wordSegment;    // the code segment

    Vector<Value>                               valueStack;     // contains values on the stack
    Vector<RetEntry>                            returnStack;    // contains calling word pointer
    Vector<Error>                               exceptionStack; // exception stack
    Vector<Value>                               localStack;     // local block stack

    Vector<IStream::Ptr>                        streams;

    Vector<Value>                               constDataSegment;   // strings, names, ...

    uint32_t                                    wp;             // instruction pointer
    uint32_t                                    lp;             // local pointer

    Signal                                      sig;

    // debugging facilites
    bool                                        verboseDebugging;


    friend struct   Primitives;
};

struct StdInStream : public IStream {
    void        testAndFillBuffer();
    uint32_t    peekChar() override;
    uint32_t    getChar() override;
    Mode        getMode() const override;
    void        setMode(Mode m) override;

    ~StdInStream() override;
    StdInStream();

    Mode        mode;
    uint32_t    pos;
    Forth::String   buff;
};

struct StringStream : public Forth::IStream {
    uint32_t        peekChar() override;
    uint32_t        getChar() override;
    Mode            getMode() const override;
    void            setMode(Mode m) override;

    StringStream(const char* str);
    ~StringStream()    override;


    Mode        mode;
    uint32_t    pos;
    Forth::String buff;
};


struct Primitives {
    // primitives
    static void     fetchInt32      (VM* vm);
    static void     returnWord      (VM* vm);
    static void     wordId          (VM* vm);
    static void     callIndirect    (VM* vm);

    static void     printInt32      (VM* vm);
    static void     printChar       (VM* vm);
    static void     defineWord      (VM* vm);
    static void     immediate       (VM* vm);
    static void     setLocalCount   (VM* vm);
    static void     addInt32        (VM* vm);
    static void     subInt32        (VM* vm);
    static void     mulInt32        (VM* vm);
    static void     divInt32        (VM* vm);
    static void     modInt32        (VM* vm);
    static void     branch          (VM* vm);
    static void     branchIf        (VM* vm);
    static void     dup             (VM* vm);
    static void     drop            (VM* vm);
    static void     swap            (VM* vm);
    static void     codeSize        (VM* vm);
    static void     endWord         (VM* vm);

    static void     emitWord        (VM* vm);
    static void     emitConstData   (VM* vm);
    static void     emitException   (VM* vm);

    static void     streamPeek      (VM* vm);
    static void     streamGetCH     (VM* vm);
    static void     streamToken     (VM* vm);

    static void     ieq             (VM* vm);
    static void     ineq            (VM* vm);
    static void     igt             (VM* vm);
    static void     ilt             (VM* vm);
    static void     igeq            (VM* vm);
    static void     ileq            (VM* vm);
    static void     notBW           (VM* vm);
    static void     andBW           (VM* vm);
    static void     orBW            (VM* vm);

    // machine stacks
    static void     vsPtr           (VM* vm);
    static void     rsPtr           (VM* vm);
    static void     wsPtr           (VM* vm);
    static void     cdsPtr          (VM* vm);
    static void     esPtr           (VM* vm);

    static void     vsFetch         (VM* vm);
    static void     rsFetch         (VM* vm);
    static void     lsFetch         (VM* vm);
    static void     wsFetch         (VM* vm);
    static void     cdsFetch        (VM* vm);
    static void     esFetch         (VM* vm);

    static void     vsStore         (VM* vm);
    static void     lsStore         (VM* vm);
    static void     wsStore         (VM* vm);
    static void     cdsStore        (VM* vm);
    static void     esStore         (VM* vm);



    static void     bye             (VM* vm);
    static void     exit            (VM* vm);

    // debug helpers
    static void     showValueStack  (VM* vm);
    static void     see             (VM* vm);
    static void     setDebugMode    (VM* vm);
};
}

#endif  // __FORTH__HPP__
