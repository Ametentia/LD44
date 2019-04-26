#if !defined(LUDUM_MATHS_H_)
#define LUDUM_MATHS_H_

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
