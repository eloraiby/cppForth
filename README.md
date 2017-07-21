# cppForth
An embeddable Forth dialect implemented in C++ (without dependencies on the STL). The binary code fits in < 32Kb for ARMv7 and < 64kb for x64 (this requires adding __cxx__ low level ABI functions). Using libstdc++ will bloat it up to 93Kb.

#### Disclaimer
This is <b>NOT</b> an <b>ANS FORTH</b> compliant implementation.

## Why a Forth interpreter ?
Implementing a barebone forth interpreter is quite straightforward, but implementing a hardened one (with debugging features and proper error handling) is quite a work. This implementation started as an experiment to drive my RPI Zero W and see how hard would it be to implement a full fledged VM. It still has a long way to go before reaching all these goals. Otherwise, I like puzzles and making Forth is like solving a puzzle for each function (= hardly productive but rewarding, your brain will thank you for it). Like functionnal programming, the concepts you learn from it, will have deep impact on how you solve problems in other languages.

## Forth in a product ?
IMHO, I would hardly see ANS Forth (the language) productive and fit for a modern environment with multicore, distributed machines. At least I know I don't have that much experience with Forth to be productive using it (information is very scarce to say the least).
I have a very hard time promoting functionnal programming languages at work, promoting Forth will be an order of magnitude more difficult.

Concurrent, Type Safe Forth with locals (Stack VM) on the other hand is another subject and it's no secret that the most performing VM platforms (.Net and JVM) are Type Safe, Concurrent Stack machines with locals.

That said, some remarkable projects used and still use Forth or a Forth dialect:
- Skia (2D compositing/rendering) used to have an embedded forth to drive it in 2009 ([addition commit](https://github.com/google/skia/commit/f56e295e88f4ed42f4c94c54d5fc544ed0f45f18)), that code perished in 2016 : [removal commit](https://github.com/google/skia/commit/acc875f9a27d3d0ece0c1b09bbc249ac69e76bac).
- FreeBSD uses it for the boot loader through FICL ([ref](https://www.freebsd.org/cgi/man.cgi?loader(8))).
- PumpkinDB uses it as a query language ([ref](http://pumpkindb.org/doc/)).
- PostScript
- ...


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
