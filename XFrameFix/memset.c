#include <crtdefs.h>

#pragma function(memset)

void * __cdecl memset(void *pTarget, int value, size_t cbTarget) {
    unsigned char *p = (unsigned char *)(pTarget);
    while (cbTarget-- > 0) {
        *p++ = (unsigned char)(value);
    }
    return pTarget;
}
