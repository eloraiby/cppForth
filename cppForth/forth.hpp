#ifndef __FORTH__HPP__
#define __FORTH__HPP__

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

#include <cfloat>
#include <cstdint>

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


    inline uint32_t wordAddr(uint32_t word)     { functions[word].start; }

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

    // primitives
    static State    fetchInt32  (VM* vm);
    static State    returnWord  (VM* vm);
    static State    wordId      (VM* vm);
    static State    printInt32  (VM* vm);
    static State    defineWord  (VM* vm);
    static State    immediate   (VM* vm);
    static State    addInt32    (VM* vm);
    static State    subInt32    (VM* vm);
    static State    mulInt32    (VM* vm);
    static State    divInt32    (VM* vm);
    static State    modInt32    (VM* vm);
    static State    branch      (VM* vm);
    static State    dup         (VM* vm);
    static State    drop        (VM* vm);
    static State    codeSize    (VM* vm);
    static State    endWord     (VM* vm);
    static State    emitWord    (VM* vm);


    std::vector<Function>                       functions;
    std::unordered_map<std::string, uint32_t>   nameToWord;

    std::vector<uint32_t>                       words;

    std::vector<Value>                          valueStack;
    std::vector<uint32_t>                       returnStack;

    std::vector<IStream::Ptr>                   streams;

    uint32_t                                    wp;     // instruction pointer
    State                                       state;
};

}

#endif  // __FORTH__HPP__