

#pragma once

#include <vector>
#include "Shader.h"

namespace PathTracer
{
    class Program
    {
    private:
        GLuint object;

    public:
        Program(const std::vector<Shader> shaders);
        ~Program();
        void Use();
        void StopUsing();
        GLuint getObject();
    };
}
