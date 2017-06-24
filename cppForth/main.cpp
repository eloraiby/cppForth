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

int
main(int argc, char* argv) {
    Forth::IStream::Ptr strm(new StdStream());
    Forth::VM*  vm  = new Forth::VM();

    vm->loadStream(strm);

    return 0;
}