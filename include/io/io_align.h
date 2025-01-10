#ifndef IO_ALIGN_H
#define IO_ALIGN_H

#include <stdint.h>

#define IO_ALIGNOF(type) offsetof(struct { char c; type member; }, member)
#define IO_ALIGNAS(align, ptr) ((void*)(((uintptr_t)(ptr) + ((align) - 1)) & ~((align) - 1)))
#define IO_ALIGNUP(n, align) (((n) + (align) - 1) & ~((align) - 1))
#define IO_ALIGNDOWN(n, align) ((n) & ~((align) - 1))

typedef long double io_max_align;

#endif
