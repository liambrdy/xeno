#ifndef MATHS_H
#define MATHS_H

#include "types.h"

struct v2 {
    f32 x, y;
};

#define V2(x, y) (v2){x, y}

v2 operator+(v2 a, v2 b)
{
    v2 result = {};
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

v2 operator-(v2 a, v2 b)
{
    v2 result = {};
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

v2 operator*(v2 a, float x)
{
    v2 result = {};
    result.x = a.x * x;
    result.y = a.y * x;
    return result;
}

bool operator>(v2 a, v2 b) {
    return a.x > b.x && a.y > b.y;
}

struct rectangle2 {
    v2 min, max;
};

bool IsInRectangle(rectangle2 rect, v2 point) {
    return point > rect.min && rect.max > point;
}

#endif