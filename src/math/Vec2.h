

#pragma once

namespace PathTracer
{
    struct iVec2
    {
    public:
        iVec2() { x = 0, y = 0; };
        iVec2(int x, int y) { this->x = x; this->y = y; };

        int x, y;
    };

    struct Vec2
    {
    public:
        Vec2() { x = 0, y = 0; };
        Vec2(float x, float y) { this->x = x; this->y = y; };
        float operator[](int i) const;

        float x, y;
    };

    inline float Vec2::operator[](int i) const
    {
        if (i == 0)
            return x;
        else if (i == 1)
            return y;
    };
}