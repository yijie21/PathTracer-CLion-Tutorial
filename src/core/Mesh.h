

#pragma once

#include <vector>
#include "split_bvh.h"

namespace PathTracer
{
    class Mesh
    {
    public:
        Mesh()
        {
            bvh = new RadeonRays::SplitBvh(2.0f, 64, 0, 0.001f, 0);
        }
        ~Mesh() { delete bvh; }

        void BuildBVH();
        bool LoadFromFile(const std::string& filename);

        std::vector<Vec4> vertexXYZU; // Vertex + texture Coord (u/s)
        std::vector<Vec4> normalXYZV;  // Normal + texture Coord (v/t)

        RadeonRays::Bvh* bvh;
        std::string name;
    };

    class MeshInstance
    {

    public:
        MeshInstance(std::string name, int meshId, Mat4 xform, int matId)
            : name(name)
            , meshID(meshId)
            , transform(xform)
            , materialID(matId)
        {
        }
        ~MeshInstance() {}

        Mat4 transform;
        std::string name;

        int materialID;
        int meshID;
    };
}
