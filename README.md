# cppForth
An embeddable Forth dialect implemented in C++ (without dependencies on the STL). The binary code fits in < 32Kb for ARMv7 and < 64kb for x64 (this requires adding __cxx__ low level ABI functions).

Implementing a barebone forth interpreter is quite straightforward, but implementing a hardened one (with debugging features and proper error handling) is quite a work. This implementation started as an experiment to drive my RPI Zero W and see how hard would it be to implement a full fledged VM. It still has a long way to go before reaching all these goals.

#### Disclaimer
This is <b>NOT</b> an <b>ANS FORTH</b> compliant implementation.

## Why a Forth interpreter ?
Simply to learn the language and use it on my RPI Zero W. I like puzzles and using it is like solving a puzzle for each function.

## Forth in a product ?
IMHO, I would hardly see Forth (the language) fit for a modern productive environment with multicore, networked machines. I know I don't have that much experience with Forth to be productive with it (information is very scarce to say the least).
I have a very hard time promoting functionnal programming languages at work, promoting Forth will be an order of magnitude more difficult.

Stack VM on the other hand are another subject and it's no secret that the most performing VM platforms are stack based ones (.Net and JVM): Stack machines are here to stay.

That said, some remarkable projects used and still use Forth or a Forth dialect:
- Skia (2D compositing/rendering) used to have an embedded forth to drive skia in 2009 ([addition commit](https://github.com/google/skia/commit/f56e295e88f4ed42f4c94c54d5fc544ed0f45f18)), that code perished long time ago : [removal commit](https://github.com/google/skia/commit/acc875f9a27d3d0ece0c1b09bbc249ac69e76bac).
- FreeBSD uses it for the boot loader using FICL ([ref](https://www.freebsd.org/cgi/man.cgi?loader(8))).
- PumpkinDB uses it as query language ([ref](http://pumpkindb.org/doc/)).
- PostScript


## Implementation Details
The implementation uses multiple stacks and segments.

* <b>Code/Word segment:</b> this is where all words are defined
* <b>Constant data segment:</b> this is where all the constant data is stored

- <b>Value stack:</b> all data passes by this stack (float, uint32 and int32)
- <b>Return stack:</b> contains the return addresses (word stack positions) for calls
- <b>Call stack:</b> for debugging purpose only. It contains the current calling words ids
- <b>Exception stack:</b> contains the stack of occuring exceptions

#### TODO
Still missing is the local stack. This will be added when all debugging features are completed.
