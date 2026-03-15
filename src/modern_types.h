#pragma once
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef uint8_t byte;

#ifndef __cplusplus
#define not !
#define and &&
#define or ||

#define logical_or |
#define logical_and &
#define logical_xor ^
#define logical_not ~
#endif

#define KiB(n) ((n) * 1024)
#define MiB(n) ((n) * 1024 * 1024)
#define GiB(n) ((n) * 1024 * 1024 * 1024)