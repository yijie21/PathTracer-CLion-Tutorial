

bool AnyHit(Ray r, float maxDist)
{

#ifdef OPT_LIGHTS
    // Intersect Emitters
    for (int i = 0; i < numOfLights; i++)
    {
        // Fetch light Data
        vec3 position = texelFetch(lightsTexture, ivec2(i * 5 + 0, 0), 0).xyz;
        vec3 emission = texelFetch(lightsTexture, ivec2(i * 5 + 1, 0), 0).xyz;
        vec3 u        = texelFetch(lightsTexture, ivec2(i * 5 + 2, 0), 0).xyz;
        vec3 v        = texelFetch(lightsTexture, ivec2(i * 5 + 3, 0), 0).xyz;
        vec3 params   = texelFetch(lightsTexture, ivec2(i * 5 + 4, 0), 0).xyz;
        float radius  = params.x;
        float area    = params.y;
        float type    = params.z;

        // Intersect rectangular area light
        if (type == QUAD_LIGHT)
        {
            vec3 normal = normalize(cross(u, v));
            vec4 plane = vec4(normal, dot(normal, position));
            u *= 1.0f / dot(u, u);
            v *= 1.0f / dot(v, v);

            float d = RectIntersect(position, u, v, plane, r);
            if (d > 0.0 && d < maxDist)
                return true;
        }

        // Intersect spherical area light
        if (type == SPHERE_LIGHT)
        {
            float d = SphereIntersect(radius, position, r);
            if (d > 0.0 && d < maxDist)
                return true;
        }
    }
#endif

    // Intersect BVH and tris
    int stack[64];
    int ptr = 0;
    stack[ptr++] = -1;

    int index = topBVHIndex;
    float leftHit = 0.0;
    float rightHit = 0.0;

#if defined(OPT_ALPHA_TEST) && !defined(OPT_MEDIUM)
    int currMatID = 0;
#endif
    bool BLAS = false;

    Ray rTrans;
    rTrans.origin = r.origin;
    rTrans.direction = r.direction;

    while (index != -1)
    {
        ivec3 LRLeaf = ivec3(texelFetch(BVHTexture, index * 3 + 2).xyz);

        int leftIndex  = int(LRLeaf.x);
        int rightIndex = int(LRLeaf.y);
        int leaf       = int(LRLeaf.z);

        if (leaf > 0) // Leaf node of BLAS
        {
            for (int i = 0; i < rightIndex; i++) // Loop through tris
            {
                ivec3 vertIndices = ivec3(texelFetch(vertexIndicesTexture, leftIndex + i).xyz);

                vec4 v0 = texelFetch(verticesTexture, vertIndices.x);
                vec4 v1 = texelFetch(verticesTexture, vertIndices.y);
                vec4 v2 = texelFetch(verticesTexture, vertIndices.z);

                vec3 e0 = v1.xyz - v0.xyz;
                vec3 e1 = v2.xyz - v0.xyz;
                vec3 pv = cross(rTrans.direction, e1);
                float det = dot(e0, pv);

                vec3 tv = rTrans.origin - v0.xyz;
                vec3 qv = cross(tv, e0);

                vec4 uvt;
                uvt.x = dot(tv, pv);
                uvt.y = dot(rTrans.direction, qv);
                uvt.z = dot(e1, qv);
                uvt.xyz = uvt.xyz / det;
                uvt.w = 1.0 - uvt.x - uvt.y;

                if (all(greaterThanEqual(uvt, vec4(0.0))) && uvt.z < maxDist)
                {
#if defined(OPT_ALPHA_TEST) && !defined(OPT_MEDIUM)
                    vec2 t0 = vec2(v0.w, texelFetch(normalsTexture, vertIndices.x).w);
                    vec2 t1 = vec2(v1.w, texelFetch(normalsTexture, vertIndices.y).w);
                    vec2 t2 = vec2(v2.w, texelFetch(normalsTexture, vertIndices.z).w);

                    vec2 texCoord = t0 * uvt.w + t1 * uvt.x + t2 * uvt.y;

                    vec4 texIDs      = texelFetch(materialsTexture, ivec2(currMatID * 8 + 6, 0), 0);
                    vec4 alphaParams = texelFetch(materialsTexture, ivec2(currMatID * 8 + 7, 0), 0);
                    
                    float alpha = texture(textureMapsArrayTexture, vec3(texCoord, texIDs.x)).a;

                    float opacity = alphaParams.x;
                    int alphaMode = int(alphaParams.y);
                    float alphaCutoff = alphaParams.z;
                    opacity *= alpha;

                    // Ignore intersection and continue ray based on alpha test
                    if (!((alphaMode == ALPHA_MODE_MASK && opacity < alphaCutoff) || 
                          (alphaMode == ALPHA_MODE_BLEND && rand() > opacity)))
                        return true;
#else
                    return true;
#endif
                }
                    
            }
        }
        else if (leaf < 0) // Leaf node of TLAS
        {
            vec4 r1 = texelFetch(transformsTexture, ivec2((-leaf - 1) * 4 + 0, 0), 0).xyzw;
            vec4 r2 = texelFetch(transformsTexture, ivec2((-leaf - 1) * 4 + 1, 0), 0).xyzw;
            vec4 r3 = texelFetch(transformsTexture, ivec2((-leaf - 1) * 4 + 2, 0), 0).xyzw;
            vec4 r4 = texelFetch(transformsTexture, ivec2((-leaf - 1) * 4 + 3, 0), 0).xyzw;

            mat4 transform = mat4(r1, r2, r3, r4);

            rTrans.origin    = vec3(inverse(transform) * vec4(r.origin, 1.0));
            rTrans.direction = vec3(inverse(transform) * vec4(r.direction, 0.0));

            // Add a marker. We'll return to this spot after we've traversed the entire BLAS
            stack[ptr++] = -1;

            index = leftIndex;
            BLAS = true;
#if defined(OPT_ALPHA_TEST) && !defined(OPT_MEDIUM)
            currMatID = rightIndex;
#endif
            continue;
        }
        else
        {
            leftHit =  AABBIntersect(texelFetch(BVHTexture, leftIndex  * 3 + 0).xyz, texelFetch(BVHTexture, leftIndex  * 3 + 1).xyz, rTrans);
            rightHit = AABBIntersect(texelFetch(BVHTexture, rightIndex * 3 + 0).xyz, texelFetch(BVHTexture, rightIndex * 3 + 1).xyz, rTrans);

            if (leftHit > 0.0 && rightHit > 0.0)
            {
                int deferred = -1;
                if (leftHit > rightHit)
                {
                    index = rightIndex;
                    deferred = leftIndex;
                }
                else
                {
                    index = leftIndex;
                    deferred = rightIndex;
                }

                stack[ptr++] = deferred;
                continue;
            }
            else if (leftHit > 0.)
            {
                index = leftIndex;
                continue;
            }
            else if (rightHit > 0.)
            {
                index = rightIndex;
                continue;
            }
        }
        index = stack[--ptr];

        // If we've traversed the entire BLAS then switch to back to TLAS and resume where we left off
        if (BLAS && index == -1)
        {
            BLAS = false;

            index = stack[--ptr];

            rTrans.origin = r.origin;
            rTrans.direction = r.direction;
        }
    }

    return false;
}