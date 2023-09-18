

#pragma once

#include <vector>
#include "Quad.h"
#include "Program.h"
#include "Vec2.h"
#include "Vec3.h"

namespace PathTracer
{
    Program* LoadShaders(const Shader::ShaderSource& vertShaderObj, const Shader::ShaderSource& fragShaderObj);

    struct RenderOptions
    {
        RenderOptions()
        {
            renderResolution = iVec2(1280, 720);
            windowResolution = iVec2(1280, 720);
            uniformLightCol = Vec3(0.3f, 0.3f, 0.3f);
            backgroundCol = Vec3(1.0f, 1.0f, 1.0f);
            tileWidth = 100;
            tileHeight = 100;
            maxDepth = 2;
            maxSpp = -1;
            RRDepth = 2;
            textureWidth = 2048;
            textureHeight = 2048;
            denoiserFrameCnt = 20;
            enableRR = true;
            enableDenoiser = false;
            enableTonemap = true;
            enableAces = false;
            openglNormalMap = true;
            enableEnvMap = false;
            enableUniformLight = false;
            hideEmitters = false;
            enableBackground = false;
            transparentBackground = false;
            independentRenderSize = false;
            enableRoughnessMollification = false;
            enableVolumeMIS = false;
            envMapIntensity = 1.0f;
            envMapRot = 0.0f;
            roughnessMollificationAmt = 0.0f;
        }

        iVec2 renderResolution;
        iVec2 windowResolution;
        Vec3 uniformLightCol;
        Vec3 backgroundCol;
        int tileWidth;
        int tileHeight;
        int maxDepth;
        int maxSpp;
        int RRDepth;
        int textureWidth;
        int textureHeight;
        int denoiserFrameCnt;
        bool enableRR;
        bool enableDenoiser;
        bool enableTonemap;
        bool enableAces;
        bool simpleAcesFit;
        bool openglNormalMap;
        bool enableEnvMap;
        bool enableUniformLight;
        bool hideEmitters;
        bool enableBackground;
        bool transparentBackground;
        bool independentRenderSize;
        bool enableRoughnessMollification;
        bool enableVolumeMIS;
        float envMapIntensity;
        float envMapRot;
        float roughnessMollificationAmt;
    };

    class Scene;

    class Renderer
    {
    protected:
        Scene* scene;
        Quad* quad;

        // Opengl buffer objects and textures for storing scene data on the GPU
        GLuint BVHBuffer;
        GLuint BVHTexture;
        GLuint vertexIndicesBuffer;
        GLuint vertexIndicesTexture;
        GLuint verticesBuffer;
        GLuint verticesTexture;
        GLuint normalsBuffer;
        GLuint normalsTexture;
        GLuint materialsTexture;
        GLuint transformsTexture;
        GLuint lightsTexture;
        GLuint textureMapsArrayTexture;
        GLuint envMapTexture;
        GLuint envMapCDFTexture;

        // FBOs
        GLuint pathTraceFBO;
        GLuint pathTraceFBOLowRes;
        GLuint accumFBO;
        GLuint outputFBO;

        // Shaders
        std::string shadersDir;
        Program* pathTraceShader;
        Program* pathTraceShaderLowRes;
        Program* outputShader;
        Program* tonemapShader;

        // Render textures
        GLuint pathTraceTextureLowRes;
        GLuint pathTraceTexture;
        GLuint accumTexture;
        GLuint tileOutputTexture[2];
        GLuint denoisedTexture;

        // Render resolution and window resolution
        iVec2 renderResolution;
        iVec2 windowResolution;

        // Variables to track rendering status
        iVec2 tile;
        iVec2 numTiles;
        Vec2 invNumTiles;
        int tileWidth;
        int tileHeight;
        int currentBuffer;
        int frameCounter;
        int sampleCounter;
        float pixelRatio;

        // Denoiser output
        Vec3* denoiserInputFramePtr;
        Vec3* frameOutputPtr;
        bool denoised;

        bool initialized;

    public:
        Renderer(Scene* scene, const std::string& shadersDir);
        ~Renderer();

        void ResizeRenderer();
        void ReloadShaders();
        void Render();
        void Present();
        void Update(float secondsElapsed);
        float GetProgress();
        int GetSampleCount();
        void GetOutputBuffer(unsigned char**, int& w, int& h);

    private:
        void InitGPUDataBuffers();
        void InitFBOs();
        void InitShaders();
    };
}