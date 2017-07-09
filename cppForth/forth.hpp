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

#include <unordered_map>

#include <string>
#include <memory>

#include <cfloat>
#include <cstdint>
#include "vector.hpp"

namespace Forth {

enum { MAX_BUFF = 1024 };

struct IStream {
    typedef std::shared_ptr<IStream>    Ptr;

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
    };

    struct Error {
        ErrorCase               errorCase;
        std::string             errorString;

        Error(ErrorCase ec, const std::string& str) : errorCase(ec), errorString(str) {}
    };

    union Value {
        uint32_t            u32;
        int32_t             i32;
        float               f32;
        void*               ptr;
        
        explicit Value(uint32_t v)  : u32(v) {}
        explicit Value(int32_t v)   : i32(v) {}
        explicit Value(float v)     : f32(v) {}
        explicit Value(void* v)     : ptr(v) {}
    };

    typedef void    (*NativeFunction)(VM* vm);

    struct Function {
        std::string         name;           // keep this even in release for debugging purpose

        bool                isImmediate;
        NativeFunction      native;
        int32_t             start;

        Function() : isImmediate(false), native(nullptr), start(-1) {}
    };

    int32_t         findWord(const std::string& name);
    void            loadStream(IStream::Ptr stream);


    inline uint32_t wordAddr(uint32_t word)     { return functions[word].start; }

    inline void     push(Value v)               { valueStack.push_back(v); }
    inline Value    top() const                 { return valueStack.back(); }
    inline void     pop()                       { valueStack.pop_back(); }

    std::string     getToken();
    void            step();
    void            runCall(uint32_t word);


    inline uint32_t emit(uint32_t word)         { uint32_t pos = static_cast<uint32_t>(words.size()); words.push_back(word); return pos; }

    uint32_t        addNativeFunction(const std::string& name, NativeFunction native, bool isImmediate);

    void            throwException(ErrorCase err, const std::string& str);

    VM();

private:
    inline void     setCall(uint32_t word)      { callStack.push_back(word); returnStack.push_back(wp); wp = functions[word].start; }
    inline void     setRet()                    { wp = returnStack.back(); returnStack.pop_back(); callStack.pop_back(); }
    inline void     setBranch(uint32_t addr)    { wp = addr; }

    uint32_t        fetch()                     { ++wp; return words[wp]; }
    
    inline IStream::Ptr     stream() const      { return streams.back(); }
    inline void     pushStream(IStream::Ptr strm)   { streams.push_back(strm); }
    inline void     popStream()                 { streams.pop_back(); }

    void            initPrimitives();

    static bool     isInt(const std::string& tok);
    static int32_t  toInt32(const std::string& tok);

    Vector<Function>                            functions;
    std::unordered_map<std::string, uint32_t>   nameToWord;

    Vector<uint32_t>                            words;          // the code segment

    Vector<Value>                               valueStack;     // contains values on the stack
    Vector<uint32_t>                            returnStack;    // contains calling word pointer
    Vector<uint32_t>                            callStack;      // contains current executing words
    Vector<Error>                               exceptionStack; // exception stack

    Vector<IStream::Ptr>                        streams;

    Vector<Value>                               constDataStack; // strings, names, ...

    uint32_t                                    wp;             // instruction pointer

    // debugging facilites
    bool                                        verboseDebugging;


    friend struct   Primitives;
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

    static void     emitReturn      (VM* vm);
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
    static void     wsFetch         (VM* vm);
    static void     cdsFetch        (VM* vm);
    static void     esFetch         (VM* vm);

    static void     vsStore         (VM* vm);
    static void     rsStore         (VM* vm);
    static void     wsStore         (VM* vm);
    static void     cdsStore        (VM* vm);
    static void     esStore         (VM* vm);

    static void     exit            (VM* vm);

    // debug helpers
    static void     showValueStack  (VM* vm);
    static void     see             (VM* vm);
    static void     setDebugMode    (VM* vm);
};
}

#endif  // __FORTH__HPP__
