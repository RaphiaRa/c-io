#ifndef IO_BASIC_ERR_H
#define IO_BASIC_ERR_H

#include <stdint.h>

typedef uint64_t io_Err;

#define IO_ERR_CODE_BITS 32
#define IO_ERR_CODE_MASK ((1ULL << IO_ERR_CODE_BITS) - 1)
#define IO_ERR_CATEGORY_SHIFT IO_ERR_CODE_BITS
#define IO_ERR_PACK(category, code) (((uint64_t)(category) << IO_ERR_CATEGORY_SHIFT) | (code))
#define IO_ERR_CODE(err) ((uint32_t)((err) & IO_ERR_CODE_MASK))
#define IO_ERR_CATEGORY(err) ((uint32_t)((err) >> IO_ERR_CATEGORY_SHIFT))

#endif
