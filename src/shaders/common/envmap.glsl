

#ifdef OPT_ENVMAP

vec2 BinarySearch(float value)
{
    ivec2 envMapResInt = ivec2(envMapRes);
    int lower = 0;
    int upper = envMapResInt.y - 1;
    while (lower < upper)
    {
        int mid = (lower + upper) >> 1;
        if (value < texelFetch(envMapCDFTexture, ivec2(envMapResInt.x - 1, mid), 0).r)
            upper = mid;
        else
            lower = mid + 1;
    }
    int y = clamp(lower, 0, envMapResInt.y - 1);

    lower = 0;
    upper = envMapResInt.x - 1;
    while (lower < upper)
    {
        int mid = (lower + upper) >> 1;
        if (value < texelFetch(envMapCDFTexture, ivec2(mid, y), 0).r)
            upper = mid;
        else
            lower = mid + 1;
    }
    int x = clamp(lower, 0, envMapResInt.x - 1);
    return vec2(x, y) / envMapRes;
}

vec4 EvalEnvMap(Ray r)
{
    float theta = acos(clamp(r.direction.y, -1.0, 1.0));
    vec2 uv = vec2((PI + atan(r.direction.z, r.direction.x)) * INV_TWO_PI, theta * INV_PI) + vec2(envMapRot, 0.0);
    
    vec3 color = texture(envMapTexture, uv).rgb;
    float pdf = Luminance(color) / envMapTotalSum;
                
    return vec4(color, (pdf * envMapRes.x * envMapRes.y) / (TWO_PI * PI * sin(theta)));
}

vec4 SampleEnvMap(inout vec3 color)
{
    vec2 uv = BinarySearch(rand() * envMapTotalSum);

    color = texture(envMapTexture, uv).rgb;
    float pdf = Luminance(color) / envMapTotalSum;

    uv.x -= envMapRot;
    float phi = uv.x * TWO_PI;
    float theta = uv.y * PI;

    if (sin(theta) == 0.0)
        pdf = 0.0;

    return vec4(-sin(theta) * cos(phi), cos(theta), -sin(theta) * sin(phi), (pdf * envMapRes.x * envMapRes.y) / (TWO_PI * PI * sin(theta)));
}

#endif