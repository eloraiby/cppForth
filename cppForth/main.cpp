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
        return buff[pos];
    }

    virtual uint32_t
    getChar() override {
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

    while(1) {
        std::cout << static_cast<char>(strm->getChar());
    }
    return 0;
}