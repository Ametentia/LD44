#if !defined(LUDUM_TYPES_H_)
#define LUDUM_TYPES_H_

#include <float.h>
#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uintptr_t umm;
typedef intptr_t  smm;

typedef float  f32;
typedef double f64;

typedef sfVector2f v2;
typedef sfVector2i v2i;
typedef sfVector3f v3;
typedef sfColor    v4i;

struct buffer {
    umm length;
    u8 *data;
};

typedef buffer string;

#define cast

#define global static
#define internal static
#define local static

#define ArrayCount(x) (sizeof(x) / sizeof((x)[0]))
#define Swap(a, b) { auto __temp = a; a = b; b = __temp; }
#define Assert(exp) do { if (!(exp)) { printf("[Assertion] %s (%s:%d)\n", #exp, __FILE__, __LINE__); asm("int3"); } } while (0)

#define U8_MAX  (0xFF)
#define U16_MAX (0xFFFF)
#define U32_MAX ((u32) -1)
#define U64_MAX ((u64) -1)

#define S8_MAX  (0x7F)
#define S16_MAX (0x7FFF)
#define S32_MAX (0x7FFFFFFF)
#define S64_MAX (0x7FFFFFFFFFFFFFFF)

#define PI32 (3.14159265358979323846264338327950288f)
#define Radians(x) ((x) * (PI32 / 180.0f))
#define Degrees(x) ((x) * (180.0f / PI32))

#endif  // LUDUM_TYPES_H_
