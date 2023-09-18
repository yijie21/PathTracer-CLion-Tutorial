

 // Much of this is from accompanying code for Ray Tracing Gems II, Chapter 14: The Reference Path Tracer
 // and was adapted for this project. See https://github.com/boksajak/referencePT for the original

#pragma once

#include "Scene.h"

namespace PathTracer
{
    class Scene;

    bool LoadGLTF(const std::string& filename, Scene* scene, RenderOptions& renderOptions, Mat4 xform, bool binary);
}