

#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include <iostream>
#include <vector>
#include "stb_image_resize.h"
#include "stb_image.h"
#include "Scene.h"
#include "Camera.h"

namespace PathTracer
{
    Scene::~Scene()
    {
        for (int i = 0; i < meshes.size(); i++)
            delete meshes[i];
        meshes.clear();

        for (int i = 0; i < textures.size(); i++)
            delete textures[i];
        textures.clear();

        if (camera)
            delete camera;

        if (sceneBvh)
            delete sceneBvh;

        if (envMap)
            delete envMap;
    };

    void Scene::AddCamera(Vec3 pos, Vec3 lookAt, float fov)
    {
        delete camera;
        camera = new Camera(pos, lookAt, fov);
    }

    int Scene::AddMesh(const std::string& filename)
    {
        int id = -1;
        // Check if mesh was already loaded
        for (int i = 0; i < meshes.size(); i++)
            if (meshes[i]->name == filename)
                return i;

        id = meshes.size();
        Mesh* mesh = new Mesh;

        printf("Loading model %s\n", filename.c_str());
        if (mesh->LoadFromFile(filename))
            meshes.push_back(mesh);
        else
        {
            printf("Unable to load model %s\n", filename.c_str());
            delete mesh;
            id = -1;
        }
        return id;
    }

    int Scene::AddTexture(const std::string& filename)
    {
        int id = -1;
        // Check if texture was already loaded
        for (int i = 0; i < textures.size(); i++)
            if (textures[i]->name == filename)
                return i;

        id = textures.size();
        Texture* texture = new Texture;

        printf("Loading texture %s\n", filename.c_str());
        if (texture->LoadTexture(filename))
            textures.push_back(texture);
        else
        {
            printf("Unable to load texture %s\n", filename.c_str());
            delete texture;
            id = -1;
        }

        return id;
    }

    int Scene::AddMaterial(const Material& material)
    {
        int id = materials.size();
        materials.push_back(material);
        return id;
    }

    void Scene::AddEnvMap(const std::string& filename)
    {
        if (envMap)
            delete envMap;

        envMap = new EnvironmentMap;
        if (envMap->LoadMap(filename.c_str()))
            printf("HDR %s loaded\n", filename.c_str());
        else
        {
            printf("Unable to load HDR\n");
            delete envMap;
            envMap = nullptr;
        }
        envMapModified = true;
        dirty = true;
    }

    int Scene::AddMeshInstance(const MeshInstance& meshInstance)
    {
        int id = meshInstances.size();
        meshInstances.push_back(meshInstance);
        return id;
    }

    int Scene::AddLight(const Light& light)
    {
        int id = lights.size();
        lights.push_back(light);
        return id;
    }

    void Scene::createTLAS()
    {
        // Loop through all the mesh Instances and build a Top Level BVH
        std::vector<RadeonRays::bbox> bounds;
        bounds.resize(meshInstances.size());

        for (int i = 0; i < meshInstances.size(); i++)
        {
            RadeonRays::bbox bbox = meshes[meshInstances[i].meshID]->bvh->Bounds();
            Mat4 matrix = meshInstances[i].transform;

            Vec3 minBound = bbox.pmin;
            Vec3 maxBound = bbox.pmax;

            Vec3 right       = Vec3(matrix[0][0], matrix[0][1], matrix[0][2]);
            Vec3 up          = Vec3(matrix[1][0], matrix[1][1], matrix[1][2]);
            Vec3 forward     = Vec3(matrix[2][0], matrix[2][1], matrix[2][2]);
            Vec3 translation = Vec3(matrix[3][0], matrix[3][1], matrix[3][2]);

            Vec3 xa = right * minBound.x;
            Vec3 xb = right * maxBound.x;

            Vec3 ya = up * minBound.y;
            Vec3 yb = up * maxBound.y;

            Vec3 za = forward * minBound.z;
            Vec3 zb = forward * maxBound.z;

            minBound = Vec3::Min(xa, xb) + Vec3::Min(ya, yb) + Vec3::Min(za, zb) + translation;
            maxBound = Vec3::Max(xa, xb) + Vec3::Max(ya, yb) + Vec3::Max(za, zb) + translation;

            RadeonRays::bbox bound;
            bound.pmin = minBound;
            bound.pmax = maxBound;

            bounds[i] = bound;
        }
        sceneBvh->Build(&bounds[0], bounds.size());
        sceneBounds = sceneBvh->Bounds();
    }

    void Scene::createBLAS()
    {
        // Loop through all meshes and build BVHs
#pragma omp parallel for
        for (int i = 0; i < meshes.size(); i++)
        {
            printf("Building BVH for %s\n", meshes[i]->name.c_str());
            meshes[i]->BuildBVH();
        }
    }

    void Scene::RebuildInstances()
    {
        delete sceneBvh;
        sceneBvh = new RadeonRays::Bvh(10.0f, 64, false);

        createTLAS();
        bvhTranslator.UpdateTLAS(sceneBvh, meshInstances);

        //Copy transforms
        for (int i = 0; i < meshInstances.size(); i++)
            transforms[i] = meshInstances[i].transform;

        instancesModified = true;
        dirty = true;
    }

    // ProcessScene accomplishes the following:
    // 1. create accelarations structures(BVH) for path-tracing
    // 2. load geometric information: vertex position/uv coordinates/mesh transforms
    // 3. load appearance information: textures
    // 4. add a camera if there is not one
    void Scene::ProcessScene()
    {
        // step 1: create bottom/top level bvhs and flatten them 
        printf("Create Bottom-level Accelaration Structure\n");
        createBLAS();                                           // Bottom means mesh-level BVH
        printf("Create Top-level Accelaration Structure\n");
        createTLAS();                                           // Top means scene-level BVH
        printf("Flattening BVH\n");
        bvhTranslator.Process(sceneBvh, meshes, meshInstances); // flatten BVH

        // step 2: load vertex indices/normals/UVs as scene parameters
        int vertexCnt = 0;
        printf("Load vertex indices/normals/UVs\n");
        for (int i = 0; i < meshes.size(); i++)
        {
            int numTriangles = meshes[i]->bvh->GetNumIndices();
            const int* triIndices = meshes[i]->bvh->GetIndices();

            for (int j = 0; j < numTriangles; j++)
            {
                int index = triIndices[j];
                int v0 = (index * 3 + 0) + vertexCnt;
                int v1 = (index * 3 + 1) + vertexCnt;
                int v2 = (index * 3 + 2) + vertexCnt;
                vertIndices.push_back(iVec3(v0, v1, v2));
            }

            vertexXYZU.insert(vertexXYZU.end(), meshes[i]->vertexXYZU.begin(), meshes[i]->vertexXYZU.end());
            normalXYZV.insert(normalXYZV.end(), meshes[i]->normalXYZV.begin(), meshes[i]->normalXYZV.end());
            vertexCnt += meshes[i]->vertexXYZU.size();
        }

        // step 3: load instance transforms as scene parameters
        printf("Copying instance transforms\n");
        transforms.resize(meshInstances.size());
        for (int i = 0; i < meshInstances.size(); i++)
            transforms[i] = meshInstances[i].transform;

        // step 4: load and resize textures as scene parameters
        if (!textures.empty())
            printf("Copying and resizing textures\n");

        int requiredWidth = renderOptions.textureWidth;
        int requiredHeight = renderOptions.textureHeight;
        int texBytes = requiredWidth * requiredHeight * 4;
        textureMapsArray.resize(texBytes * textures.size());

#pragma omp parallel for        // use OpenMP to parallelize this loop
        for (int i = 0; i < textures.size(); i++)
        {
            int texWidth = textures[i]->width;
            int texHeight = textures[i]->height;

            // Resize textures to fit 2D texture array
            if (texWidth != requiredWidth || texHeight != requiredHeight)
            {
                unsigned char* resizedTex = new unsigned char[texBytes];
                stbir_resize_uint8(&textures[i]->texData[0], texWidth, texHeight, 0, resizedTex, requiredWidth, requiredHeight, 0, 4);
                std::copy(resizedTex, resizedTex + texBytes, &textureMapsArray[i * texBytes]);
                delete[] resizedTex;
            }
            else
                std::copy(textures[i]->texData.begin(), textures[i]->texData.end(), &textureMapsArray[i * texBytes]);
        }

        // step 5: add a default camera
        if (!camera)
        {
            RadeonRays::bbox bounds = sceneBvh->Bounds();
            Vec3 extents = bounds.extents();
            Vec3 center = bounds.center();
            AddCamera(Vec3(center.x, center.y, center.z + Vec3::Length(extents) * 2.0f), center, 45.0f);
        }

        initialized = true;
    }
}