

#pragma once

#include "Config.h"

namespace PathTracer
{
    class Program;

    class Quad
    {
    public:
        Quad();
        void Draw(Program*);

    private:
        GLuint vao;
        GLuint vbo;
    };
}