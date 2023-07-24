#ifndef MATHS_H
#define MATHS_H

#include "types.h"

#include <math.h>

struct v2 {
    f32 x, y;
};

inline v2 V2(f32 x, f32 y)
{
    v2 result;

    result.x = x;
    result.y = y;

    return result;
}

inline v2 operator*(f32 a, v2 b)
{
    v2 result;

    result.x = a * b.x;
    result.y = a * b.y;

    return result;
}

inline v2 operator*(v2 b, f32 a)
{
    v2 result;

    result.x = a * b.x;
    result.y = a * b.y;

    return result;
}

inline v2 &operator*=(v2 &a, f32 b)
{
    a = b * a;

    return a;
}

inline v2 operator-(v2 a)
{
    v2 result;

    result.x = -a.x;
    result.y = -a.y;

    return result;
}

inline v2 operator+(v2 a, v2 b)
{
    v2 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

inline v2 &operator+=(v2 &a, v2 b)
{
    a = a + b;

    return a;
}

inline v2 operator-(v2 a, v2 b)
{
    v2 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

bool operator>(v2 a, v2 b) {
    return a.x > b.x && a.y > b.y;
}

float Length(v2 a) {
    return sqrtf(a.x*a.x + a.y*a.y);
}

struct rectangle2 {
    v2 min, max;
};

bool IsInRectangle(rectangle2 rect, v2 point) {
    return point > rect.min && rect.max > point;
}

#endif