#pragma once

#include "Scene.h"

namespace PathTracer
{
    class Scene;

    bool LoadSceneFromFile(const std::string& filename, Scene* scene, RenderOptions& renderOptions);
}