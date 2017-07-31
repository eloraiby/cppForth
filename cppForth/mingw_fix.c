#ifdef __GNUC__
#include <stdio.h>

void
__gxx_personality_v0() {}

void
__cxa_throw_bad_array_new_length() {
}

/* debug only */
void __cxa_pure_virtual () {
    fprintf(stderr, "pure virtual called\n");
}

#endif
