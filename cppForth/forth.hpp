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
    enum class Error {
        NO_ERROR        = 0x00000000,
        WORD_NOT_FOUND  = 0xFFFFFFFF
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

    typedef bool    (*NativeFunction)(VM* vm);

    struct Function {
        bool                isImmediate;
        NativeFunction      native;
        uint32_t            start;

        Function() : isImmediate(false), native(nullptr), start(0) {}
    };

    uint32_t        allocateWord(const std::string& name);
    uint32_t        findWord(const std::string& name);
    void            loadStream(IStream::Ptr stream);

    inline void     call(uint32_t word)     { returnStack.push_back(++wp); wp = functions[word].start; }
    inline void     ret()                   { wp = returnStack.back(); returnStack.pop_back(); }
    inline void     branch(uint32_t addr)   { wp = addr; }

    inline uint32_t wordAddr(uint32_t word) { functions[word].start; }

    uint32_t        fetch()                 { return words[wp++]; }

    inline void     push(Value v)           { valueStack.push_back(v); }
    inline Value    top() const             { return valueStack.back(); }
    inline void     pop()                   { valueStack.pop_back(); }

    inline IStream::Ptr     stream() const  { return streams.back(); }
    inline void     pushStream(IStream::Ptr strm)   { streams.push_back(strm); }
    inline void     popStream()             { streams.pop_back(); }
    
    std::string     getToken();

    static bool     isInt(const std::string& tok);
    static uint32_t toUInt32(const std::string& tok);

    inline uint32_t emit(uint32_t word)     { uint32_t pos = static_cast<uint32_t>(words.size()); words.push_back(word); return pos; }

    std::vector<Function>                       functions;
    std::unordered_map<std::string, uint32_t>   nameToWord;

    std::vector<uint32_t>                       words;

    std::vector<Value>                          valueStack;
    std::vector<uint32_t>                       returnStack;

    std::vector<IStream::Ptr>                   streams;

    uint32_t                                    wp;     // instruction pointer
};

}

#endif  // __FORTH__HPP__