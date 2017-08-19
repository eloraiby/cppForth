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

#ifndef __SM_BASE__
#   include "base.hpp"
#endif

#include "intrusive-ptr.hpp"
#include "vector.hpp"
#include "string.hpp"
#include "hash_map.hpp"
#include "vm.hpp"

namespace Forth {

enum { MAX_BUFF = 1024 };

struct IInputStream : public SM::RCObject {
    typedef SM::IntrusivePtr<IInputStream>  Ptr;
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
    SM::String      buff;
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
    SM::String      buff;
};

struct Terminal : public SM::VM::Process {
    typedef SM::IntrusivePtr<Terminal>  Ptr;

    enum ErrorCase {
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

    static void     wordId          (SM::VM::Process* proc);
    static void     defineWord      (SM::VM::Process* proc);
    static void     immediate       (SM::VM::Process* proc);
    static void     setLocalCount   (SM::VM::Process* proc);
    static void     endWord         (SM::VM::Process* proc);
    static void     see             (SM::VM::Process* proc);
    static void     streamPeek      (SM::VM::Process* proc);
    static void     streamGetCH     (SM::VM::Process* proc);
    static void     streamToken     (SM::VM::Process* proc);

    void            loadStream(IInputStream::Ptr stream);

    Terminal(SM::VM* vm);

private:
    SM::String      getToken();

    inline IInputStream::Ptr    stream() const  { return streams_.back(); }
    inline void     pushStream(IInputStream::Ptr strm)  { streams_.push_back(strm); }
    inline void     popStream()                 { streams_.pop_back(); }

    SM::Vector<IInputStream::Ptr>   streams_;

    static bool     isInt(const SM::String& tok);
    static int32_t  toInt32(const SM::String& tok);
};


}

#endif  // __FORTH__HPP__
