#if !defined(LUDUM_MATHS_H_)
#define LUDUM_MATHS_H_

#include <math.h>
#include <stdlib.h>

#define Atan2(y, x) atan2f(y, x)
#define Cos(x) cosf(x)
#define Sin(x) sinf(x)
#define Tan(x) tanf(x)
#define Sqrt(x) sqrtf(x)
#define Clamp(x, min, max) Max(min, Min(x, max))

inline s32 random(s32 min, s32 max) {
	return (rand() % (min - max)) + min;
}

inline v2 V2(f32 x, f32 y) {
    v2 result = { x, y };
    return result;
}

inline v2i V2i(s32 x, s32 y) {
    v2i result = { x, y };
    return result;
}

inline v2 operator+(v2 a, v2 b) {
    v2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

inline v2 &operator+=(v2 &a, v2 b) {
    a = a + b;
    return a;
}

inline v2 operator-(v2 a, v2 b) {
    v2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

inline v2 operator-(v2 v) {
    v2 result;
    result.x = -v.x;
    result.y = -v.y;

    return result;
}

inline v2 &operator-=(v2 &a, v2 b) {
    a = a - b;
    return a;
}

inline v2 operator*(f32 s, v2 v) {
    v2 result;
    result.x = s * v.x;
    result.y = s * v.y;

    return result;
}

inline v2 operator*(v2 v, f32 s) {
    v2 result = s * v;
    return result;
}

inline v2 &operator*=(v2 &v, f32 s) {
    v = s * v;
    return v;
}

inline f32 Dot(v2 a, v2 b) {
    f32 result = (a.x * b.x) + (a.y * b.y);
    return result;
}

inline f32 Length(v2 v) {
    f32 result = Sqrt(Dot(v, v));
    return result;
}

// @Note: Returns the vector [0, 0] if the length is 0
inline v2 Normalise(v2 v) {
    v2 result = {};
    f32 length = Length(v);
    if (length != 0) {
        result = (1.0f / length) * v;
    }

    return result;
}

inline v2 Perp(v2 v) {
    v2 result = { -v.y, v.x };
    return result;
}

inline v4i CreateColour(f32 r, f32 g, f32 b, f32 a) {
    v4i result = {
        cast(u8) (255 * r),
        cast(u8) (255 * g),
        cast(u8) (255 * b),
        cast(u8) (255 * a)
    };

    return result;
}

inline v4i CreateColour(f32 r, f32 g, f32 b) {
    v4i result = CreateColour(r, g, b, 1);
    return result;
}

#endif  // LUDUM_MATHS_H_
