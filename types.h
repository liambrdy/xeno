#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#define Kilobytes(x) (x*1024)
#define Megabytes(x) (Kilobytes(x)*1024)
#define Gigabytes(x) (Megabytes(x)*1024)

#define ArrayCount(arr) (sizeof(arr)/sizeof(arr[0]))

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

#endif