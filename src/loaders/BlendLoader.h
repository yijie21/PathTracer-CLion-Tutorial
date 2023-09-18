#pragma once

#include "fbtBlend.h"
#include "Scene.h"

namespace PathTracer
{
    class Scene;

    struct BlenderModifierMirror {
        enum Flag {
            CLIPPING         =               1<<0,
            MIRROR_U         =               1<<1,
            MIRROR_V         =               1<<2,
            AXIS_X           =               1<<3,
            AXIS_Y           =               1<<4,
            AXIS_Z           =               1<<5,
            VGROUP           =               1<<6,
            DONT_MERGE       =               1<<7
        };
        bool enabled,clipping,mirrorU,mirrorV,axisX,axisY,axisZ,vGroup,merge;
        float tolerance;
        BlenderModifierMirror(const Blender::MirrorModifierData& md)
            : enabled(true),clipping(md.flag&CLIPPING),mirrorU(md.flag&MIRROR_U),mirrorV(md.flag&MIRROR_V),
            axisX(md.flag&AXIS_X),axisY(md.flag&AXIS_Y),axisZ(md.flag&AXIS_Z),vGroup(md.flag&VGROUP),merge(!(md.flag&DONT_MERGE)),
            tolerance(md.tolerance)
        {}
        BlenderModifierMirror() : enabled(false) {}
        void display() const {
            if (enabled) printf("axisX:%d axisY:%d axisZ:%d tolerance:%1.5f clipping:%d mirrorU:%d mirrorV:%d vGroup:%d merge:%d\n",(int)axisX,(int)axisY,(int)axisZ,tolerance,(int)clipping,(int)mirrorU,(int)mirrorV,(int)vGroup,(int)merge);
            else printf("enabled:false\n");
        }
        int getNumVertsMultiplier() const {
            int m(1);
            if (enabled) {
                if (axisX) m+=1;
                if (axisY) m+=1;
                if (axisZ) m+=1;
                if (m==3) return 4;
                if (m==4) return 8;
            }
            return m;
        }
        inline const float* mirrorUV(const float* tc2,float* tcOut2) const {
            if (mirrorU) tcOut2[0] = -tc2[0];
            else tcOut2[0] = tc2[0];
            if (mirrorV) tcOut2[1] = -tc2[1];
            else tcOut2[1] = tc2[1];
            return tcOut2;
        }
    };

    enum BlenderObjectType {
        BL_OBTYPE_EMPTY         =  0,
        BL_OBTYPE_MESH          =  1,
        BL_OBTYPE_CURVE         =  2,
        BL_OBTYPE_SURF          =  3,
        BL_OBTYPE_FONT          =  4,
        BL_OBTYPE_MBALL         =  5,
        BL_OBTYPE_LAMP          = 10,
        BL_OBTYPE_CAMERA        = 11,
        BL_OBTYPE_WAVE          = 21,
        BL_OBTYPE_LATTICE       = 22,
        BL_OBTYPE_ARMATURE      = 25
    };

    bool LoadBlendFromFile(const std::string& filename, Scene* scene, RenderOptions& renderOptions);
}