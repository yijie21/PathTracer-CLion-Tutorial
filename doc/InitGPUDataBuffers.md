# InitGPUDataBuffers

This `md` reveals the details of the code in `src/core/Renderer.cpp->InitGPUDataBuffers`:



### 1. Set pixel alignment mode

```c++
glPixelStorei(GL_PACK_ALIGNMENT, 1)
```

`GL_PACK_ALIGNMENT=1` means pixels are tighly packed with no padding. And this is usually set as default because textures are saves as regular square/rectangle images, which actually do not need padding.



### 2. Generate/Bind/Initialize Buffers/Textures

There are two types of code setting for `GL_TEXTURE_BUFFER` and `GL_TEXTURE_2D`.

#### GL_TEXTURE_BUFFER

This is the OpenGL buffer data structure which is frequently changed and accessed by shaders. So it needs a `buffer` served as data source for `texture`.

And this is set for `BVHTexture`, `vertexIndicesTexture`, `verticesTexure` and `normalsTexture`.

```c++
glGenBuffers(1, &BVHBuffer);
glBindBuffer(GL_TEXTURE_BUFFER, BVHBuffer);
glBufferData(GL_TEXTURE_BUFFER, 
             sizeof(RadeonRays::BvhTranslator::Node) * 
             scene->bvhTranslator.nodes.size(), 
             &scene->bvhTranslator.nodes[0], GL_STATIC_DRAW);
glGenTextures(1, &BVHTexture);
glBindTexture(GL_TEXTURE_BUFFER, BVHTexture);
glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, BVHBuffer);
glBindBuffer(GL_TEXTURE_BUFFER, 0);
glBindTexture(GL_TEXTURE_2D, 0);
```

line 1: Generate `1` buffer object and assign to `BVHBuffer`.

line 2: Binds `BVHBuffer` to `GL_TEXTURE_BUFFER`.

line 3-6: Initialize data for `GL_TEXTURE_BUFFER` using `scene->bvhTranslator.nodes`. `GL_STATIC_DRAW` hints the data is not expected to change frequently.

line 7-8: Generate/Bind `BVHTexture`

line 9: Associates the buffer `BVHBuffer` with the texture object `BVHTexture`. The data source of `BVHTexture` is `BVHBuffer`.

line 10-11: Unbind buffer/texture, add these two lines after each gen/bind/initialize.



#### GL_TEXTURE_2D

This is a static OpenGL texture objects, usually set for those image textures binded with shapes.

And this is set for `materialsTexture`, `transformsTexture`, `lightsTexture`, `textureMapsArrayTexture`, `envMapTexture` and `envMapCDFTexture`.

```c++
glGenTextures(1, &materialsTexture);
glBindTexture(GL_TEXTURE_2D, materialsTexture);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 
             (sizeof(Material) / sizeof(Vec4)) * scene->materials.size(), 
             1, 0, GL_RGBA, GL_FLOAT, &scene->materials[0]);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glBindTexture(GL_TEXTURE_2D, 0);
```

line 1-2: Generate/Bind `materialsTexture`.

line 3: Initializes the `GL_TEXTURE_2D` with `scene->materials`.

- `GL_TEXTURE_2D`: Indicates that you're working with a 2D texture.
- `0`: Specifies the level-of-detail for mipmapping (level 0 is the base level).
- `GL_RGBA32F`: Specifies the internal format of the texture. In this case, it's a 32-bit floating-point format with four channels (R, G, B, and A).
- `(sizeof(Material) / sizeof(Vec4)) * scene->materials.size()`: This calculates the width of the texture in pixels based on the size of `Material` and `Vec4` (assuming that `Material` is a structure with the same layout as `Vec4`). It multiplies this width by the number of materials in the `scene` to determine the total width of the texture.
- `1`: The height of the texture (1 in this case).
- `0`: The border of the texture (0 means no border).
- `GL_RGBA`: Specifies the format of the incoming data (in this case, RGBA).
- `GL_FLOAT`: Specifies that the incoming data is in floating-point format.
- `&scene->materials[0]`: This provides a pointer to the data that will be used to initialize the texture. It assumes that `scene->materials` is an array of structures or vectors containing RGBA color data.

line 6-7: Interpolation mode is `nearest` when enlarging or shrinking textures.

line 8: Unbind texture.



### 3. Bind Textures to Different Slots

Before this, each texture is operated on `GL_TEXTURE0`, so this is going to bind each textures to slots from 1-10.

```c++
glActiveTexture(GL_TEXTURE1);
glBindTexture(GL_TEXTURE_BUFFER, BVHTexture);
```

line 1: Activate `GL_TEXTURE1`

line 2: Bind `BVHTexture` to `GL_TEXTURE_BUFFER`

