#include "BlendLoader.h"

namespace PathTracer
{
    void LoadMaterial(const Blender::Material* ma, Material& material)
    {
        material.baseColor = Vec3(ma->r * ma->ref, ma->g * ma->ref, ma->b * ma->ref);
        material.emission = Vec3(ma->r * ma->emit, ma->g * ma->emit, ma->b * ma->emit);
        material.opacity = ma->alpha;
        material.roughness = ma->roughness;
    }

    bool LoadBlendFromFile(const std::string& filename, Scene* scene, RenderOptions& renderOptions)
    {
        fbtBlend blendfile;
        if (blendfile.parse(filename.c_str()) != fbtFile::FS_OK) {
            printf("ERROR: \"%s\" does not exist, or can't be loaded.\n", filename.c_str());
            return false;
        } else {
            printf("File \"%s\" loaded successfully.\n", filename.c_str());
        }

        fbtList& objects = blendfile.m_object;
        long cnt=-1;
        
        for (Blender::Object* ob = (Blender::Object*)objects.first; ob; ob = (Blender::Object*)ob->id.next)	{
            ++cnt;

            // if not a mesh, continue
            if (!ob->data || ob->type != (short) BL_OBTYPE_MESH) {
                continue;
            }

            // check if the mesh has mirror modifiers or not:
            BlenderModifierMirror mirrorModifier;
            const Blender::ModifierData* mdf = (const Blender::ModifierData*) ob->modifiers.first;
            while (mdf) {
                if (mdf->error) {
                    printf("Error: %s\n",mdf->error);
                } else {
                    if (mdf->type == 5) {
                        const Blender::MirrorModifierData* md = (const Blender::MirrorModifierData*) mdf;
                        if (!mirrorModifier.enabled && !md->mirror_ob) {
                            mirrorModifier = BlenderModifierMirror(*md);
                        }
                    } else {
                        fprintf(stderr,"Detected unknown modifier: %d for object: %s\n",mdf->type,ob->id.name);
                    }
                }
                mdf = mdf->next;    // see next modifier
            }

            const Blender::Mesh* me = (const Blender::Mesh*)ob->data;
            Mesh* mesh = new Mesh();

            // VALIDATION-------------------------------------------
            printf("\tMesh Name: %s\n", me->id.name);
            const bool hasFaces = me->totface>0 && me->mface;
            const bool hasPolys = me->totpoly>0 && me->mpoly && me->totloop>0 && me->mloop;
            const bool hasVerts = me->totvert>0;
            const bool isValid = hasVerts && (hasFaces || hasPolys);
            if (!isValid) continue;
            //-------------------------------------------------------

            int numOfVertices = me->totvert;
            const float MAX_SHORT = 32767;                 //INT16_MAX

            for (int i = 0; i < numOfVertices; i++) {
                const Blender::MVert& v = me->mvert[i];
                const Blender::MLoopUV& loopuv = me->mloopuv[i];
                Vec3 pos(v.co[0], v.co[2], -v.co[1]);
                Vec3 normal(v.no[0] / MAX_SHORT, v.no[2] / MAX_SHORT, -v.no[1] / MAX_SHORT);
                Vec2 uv(0, 1);
                normal = Vec3::Normalize(normal);

                mesh->vertexXYZU.push_back(Vec4(pos[0], pos[1], pos[2], uv[0]));
                mesh->normalXYZV.push_back(Vec4(normal[0], normal[1], normal[2], uv[1]));
            }
            mesh->name = std::string(me->id.name);
            scene->meshes.push_back(mesh);

            Mat4 transform;
            for (int i=0; i<4; i++) {
                for (int j=0; j<4; j++) {
                    if (i == 0) transform.data[j][i] = ob->obmat[j][i];
                    if (i == 1) transform.data[j][i] = ob->obmat[j][2];
                    if (i == 2) transform.data[j][i] = -ob->obmat[j][1];
                }
            }
            MeshInstance instance(std::string(me->id.name), scene->meshes.size() - 1,
                                  transform, scene->meshes.size());
            scene->AddMeshInstance(instance);

            // MATERIALS---------------------------------------------
            // Load Materials first
            // Original materials are always present in me->mat, but they can be overridden in ob->mat
            const bool areMaterialsOverridden = ob->mat && *ob->mat;
            Blender::Material **mat = areMaterialsOverridden ? ob->mat : me->mat;
            const short totcol = areMaterialsOverridden ? ob->totcol : me->totcol;  // However we assume ob->totcol==me->totcol (don't know what we should do otherwise)
            if (mat && *mat) {
                for (int i = 0; i < ob->totcol; ++i) {
                    Material material;
                    const Blender::Material* ma =  mat[i];
                    if (!ma) continue;
                    LoadMaterial(ma, material);
                    scene->AddMaterial(material);
                }
            }
        }

        return true;
    }
}