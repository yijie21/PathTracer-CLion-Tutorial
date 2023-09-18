

#include <iostream>
#include <fstream>
#include <sstream>
#include "Shader.h"

namespace PathTracer
{
    Shader::Shader(const ShaderSource& shaderSrc, GLenum shaderType)
    {
        // Adding shader process
        // step 1: create a shader (glCreateShader)
        // step 2: specify the source code for a shader (glShaderSource)
        // step 3: compile shader (glCompileShader)
        // step 4: check if compilation is successful (glGetShaderiv)
        object = glCreateShader(shaderType);
        printf("Compiling Shader %s\n", shaderSrc.path.c_str());
        const GLchar* src = (const GLchar*)shaderSrc.src.c_str();
        glShaderSource(object, 1, &src, 0);
        glCompileShader(object);
        GLint success = 0;
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE)
        {
            std::string msg;
            GLint logSize = 0;
            glGetShaderiv(object, GL_INFO_LOG_LENGTH, &logSize);
            char* info = new char[logSize + 1];
            glGetShaderInfoLog(object, logSize, NULL, info);
            msg += shaderSrc.path;
            msg += "\n";
            msg += info;
            delete[] info;
            glDeleteShader(object);
            object = 0;
            printf("Shader compilation error %s\n", msg.c_str());
            throw std::runtime_error(msg.c_str());
        }
    }

    GLuint Shader::getObject() const
    {
        return object;
    }
}