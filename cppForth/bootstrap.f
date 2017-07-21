: _sink_ 0 0 0 0 ;

: i32>w ' lit.i32 w> w> ;

: do immediate
    w& 1 + ;

: while immediate
    i32>w
    ' ?branch w> ;

: ( immediate
    do
        stream.getch dup
        0 =/=
        swap
        41 =/=
        and
    while ;

: \ immediate
    do
        stream.getch dup
        0 =/=
        swap
        10 =/=
        and
    while ; 

: if immediate ( cond -- )
    w& 7 + i32>w   \ then addr
    ' ?branch w>
    w& 2 +
    0 i32>w         \ else addr
    ' branch w> ;

: then immediate
    w& 1 + swap w! ;
    
: else immediate
    w& 4 + swap w! \ set the else addr in IF after the coming branch
    w& 2 +
    0 i32>w
    ' branch w> ;

: .readString
    cd& 1 + i32>w                   \ --
    do
        stream.getch dup dup        \ -- c c c
        0 =/=                       \ -- c c b
        swap                        \ -- c b c
        34 =/=                      \ -- c b b
        and                         \ -- c b
        dup                         \ -- c b b
        if                          \ -- c b
            swap                    \ -- b c
            cd>                     \ -- b
        then                        \ -- b
    while                           \ c b -- c
    drop                            \ c --
    0 cd> ;                         \ null terminate


: " immediate
    .readString ;

: .cd                               \ -- c-addr
    do      
        dup                         \ -- c-addr c-addr
        cd@                         \ -- c-addr c
        dup .c                      \ -- c-addr c
        swap                        \ -- c c-addr
        1 +                         \ -- c c-addr
        swap                        \ -- c-addr c
    while                           \ -- c-addr
    drop ;                          \ --


: ." immediate
    cd& 1 +
    .readString
    .cd ;

        
: *2 2 * ;

: /2 2 / ;

: +1 1 + ;

: -1 1 - ;

: testifthen 0 == if 123 . then 456 . ;

: testifthenelse 0 == if 123 . else 456 . then 789 . ;

: dec100
    100
    do
        dup
        .
        1 -
        dup
        0 =/=
    while
    drop ;
