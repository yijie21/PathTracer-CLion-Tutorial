uniform bool isCameraMoving;
uniform vec3 randomVector;
uniform vec2 resolution;
uniform vec2 tileOffset;
uniform vec2 invNumTiles;

uniform sampler2D accumTexture;
uniform samplerBuffer BVHTexture;
uniform isamplerBuffer vertexIndicesTexture;
uniform samplerBuffer verticesTexture;
uniform samplerBuffer normalsTexture;
uniform sampler2D materialsTexture;
uniform sampler2D transformsTexture;
uniform sampler2D lightsTexture;
uniform sampler2DArray textureMapsArrayTexture;

uniform sampler2D envMapTexture;
uniform sampler2D envMapCDFTexture;

uniform vec2 envMapRes;
uniform float envMapTotalSum;
uniform float envMapIntensity;
uniform float envMapRot;
uniform int numOfLights;
uniform int maxDepth;
uniform int topBVHIndex;
uniform int frameNum;