: i32>w ' lit.i32 >w >w ;

: w.p+ w.p 1 + ;

: repeat immediate
    w.p+ ; 

: until immediate
    i32>w
    ' ?branch >w ;

: ( immediate
    repeat
        stream.getch dup
        0 =/=
        swap
        41 =/=
        and
    until ;

: \ immediate
    repeat
        stream.getch dup
        0 =/=
        swap
        10 =/=
        and
    until ; 

: if immediate ( cond -- )
    w.p+ 6 + i32>w
    ' ?branch >w
    w.p 2 +
    0 i32>w
    ' branch >w ; 

: then immediate
    w.p+ swap !w ;
    
: else ; immediate

: .readString
    cd.p 1 + i32>w
    repeat
        stream.getch dup dup dup
        0 =/=
        swap
        34 =/=
        and
        dup
        if
            dup 10 .c swap .c 10 .c
            >cd
        then
    until
    0 >cd ; \ null terminate

: " immediate
    .readString ;

: .cd
    repeat
        dup
        @cd
        dup .c
        swap
        1 +
        swap
    until ;

: ." immediate
    .readString
    ' .cd >w ;

        
: *2 2 * ;

: /2 2 / ;

: +1 1 + ;

: -1 1 - ;

: testif 0 == if 123 . then 456 . ;

: dec100
    100
    repeat
        dup
        .
        1 -
    until ;
