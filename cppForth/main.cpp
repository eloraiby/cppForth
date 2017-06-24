#include <iostream>
#include "forth.hpp"

struct StdStream : public Forth::IStream {
    void
    testAndFillBuffer() {
        if(pos >= buff.size()) {
            std::string tmp;
            std::cin >> tmp;
            tmp += '\n';
            buff += tmp;
        }
    }

    virtual uint32_t
    peekChar() override {
        testAndFillBuffer();
        return buff[pos];
    }

    virtual uint32_t
    getChar() override {
        testAndFillBuffer();
        return buff[pos++];
    }
    
    virtual Mode
    getMode() const override {
        return mode;
    }
    
    virtual void
    setMode(Mode m) override {
        mode = m;
    }

    ~StdStream()    override {
    }

    StdStream() : mode(Mode::EVAL), pos(0) {}


    Mode        mode;
    uint32_t    pos;
    std::string buff;
};

struct StringStream : public Forth::IStream {

    StringStream(const char* str) : mode(Mode::EVAL), pos(0), buff(str) {}

    virtual uint32_t
    peekChar() override {
        if( pos == buff.size() ) {
            return 0;
        } else {
            return buff[pos];
        }
    }

    virtual uint32_t
    getChar() override {
        if( pos == buff.size() ) {
            return 0;
        } else {
            return buff[pos++];
        }
    }
    
    virtual Mode
    getMode() const override {
        return mode;
    }
    
    virtual void
    setMode(Mode m) override {
        mode = m;
    }

    ~StringStream()    override {
    }


    Mode        mode;
    uint32_t    pos;
    std::string buff;
};

const char
core[] =
    ": *2 2 * ;\n" \
    ": /2 2 / ;\n" \
    ": here code.size 1 - ; immediate\n"
    ;

int
main(int argc, char* argv) {
    Forth::VM*  vm  = new Forth::VM();

    Forth::IStream::Ptr coreStream(new StringStream(core));
    vm->loadStream(coreStream);

    Forth::IStream::Ptr strm(new StdStream());
    vm->loadStream(strm);

    return 0;
}