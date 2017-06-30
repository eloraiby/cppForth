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

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

#include <cfloat>
#include <cstdint>

//#define _DEBUG

namespace Forth {

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
            || ch == static_cast<uint32_t>('\t')
            || ch == static_cast<uint32_t>(' ')
            || ch == static_cast<uint32_t>('\a'));
    }
};

struct VM {
    enum class State {
        NO_ERROR        =  0,
        WORD_NOT_FOUND  = -1,
        HALT            = -2,
        VS_UNDERFLOW    = -3,
        VS_OVERFLOW     = -4,
        RS_UNDERFLOW    = -5,
        RS_OVERFLOW     = -6,
        WORD_NOT_DEFINED= -7,
        INT_IS_NO_WORD  = -8,
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

    typedef State   (*NativeFunction)(VM* vm);

    struct Function {
#ifdef _DEBUG
        std::string         name;
#endif
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
    inline void     setState(State s)           { state = s; }
    inline State    getState() const            { return state; }

    uint32_t        addNativeFunction(const std::string& name, NativeFunction native, bool isImmediate);

    VM();

private:
    inline void     setCall(uint32_t word)      { returnStack.push_back(wp); wp = functions[word].start; }
    inline void     setRet()                    { wp = returnStack.back(); returnStack.pop_back(); }
    inline void     setBranch(uint32_t addr)    { wp = addr; }

    uint32_t        fetch()                     { ++wp; return words[wp]; }
    
    inline IStream::Ptr     stream() const      { return streams.back(); }
    inline void     pushStream(IStream::Ptr strm)   { streams.push_back(strm); }
    inline void     popStream()                 { streams.pop_back(); }

    void            initPrimitives();

    static bool     isInt(const std::string& tok);
    static int32_t  toInt32(const std::string& tok);

    std::vector<Function>                       functions;
    std::unordered_map<std::string, uint32_t>   nameToWord;

    std::vector<uint32_t>                       words;

    std::vector<Value>                          valueStack;
    std::vector<uint32_t>                       returnStack;

    std::vector<IStream::Ptr>                   streams;

    uint32_t                                    wp;     // instruction pointer
    State                                       state;

    friend struct   Primitives;
};

struct Primitives {
    // primitives
    static VM::State    fetchInt32  (VM* vm);
    static VM::State    returnWord  (VM* vm);
    static VM::State    wordId      (VM* vm);
    static VM::State    printInt32  (VM* vm);
    static VM::State    defineWord  (VM* vm);
    static VM::State    immediate   (VM* vm);
    static VM::State    addInt32    (VM* vm);
    static VM::State    subInt32    (VM* vm);
    static VM::State    mulInt32    (VM* vm);
    static VM::State    divInt32    (VM* vm);
    static VM::State    modInt32    (VM* vm);
    static VM::State    branch      (VM* vm);
    static VM::State    branchIf    (VM* vm);
    static VM::State    dup         (VM* vm);
    static VM::State    drop        (VM* vm);
    static VM::State    swap        (VM* vm);
    static VM::State    codeSize    (VM* vm);
    static VM::State    endWord     (VM* vm);
    static VM::State    emitWord    (VM* vm);
    static VM::State    streamPeek  (VM* vm);
    static VM::State    streamGetCH (VM* vm);
    static VM::State    streamToken (VM* vm);
    static VM::State    ieq         (VM* vm);
    static VM::State    ineq        (VM* vm);
    static VM::State    igt         (VM* vm);
    static VM::State    ilt         (VM* vm);
    static VM::State    igeq        (VM* vm);
    static VM::State    ileq        (VM* vm);
    static VM::State    pick        (VM* vm);

    static VM::State    exit        (VM* vm);
};
}

#endif  // __FORTH__HPP__
