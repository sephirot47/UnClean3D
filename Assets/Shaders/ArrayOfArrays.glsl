#ifndef ARRAY_OF_ARRAYS_GLSL
#define ARRAY_OF_ARRAYS_GLSL

vec4 _GetArrayOfArraysPixel(sampler2D arrayOfArraysTexture,
                            vec2 arrayOfArraysTextureSize,
                            vec2 arrayOfArraysTexelUvSize,
                            int index)
{
    vec2 texCoord = vec2(mod(index, arrayOfArraysTextureSize.x),
                         index / arrayOfArraysTextureSize.x);
    vec2 uvs = texCoord * arrayOfArraysTexelUvSize;
    return texture(arrayOfArraysTexture, uvs, 0);
}

#define ARRAY_OF_ARRAYS(ArrayOfArraysName) \
    uniform sampler2D ArrayOfArraysName; \
    uniform int  ArrayOfArraysName##_HeaderSize; \
    uniform vec2 ArrayOfArraysName##_Size; \
    uniform vec2 ArrayOfArraysName##_TexelUvSize; \
    \
    int Get##ArrayOfArraysName##ArraySize(int outerArrayIndex) \
    { \
        vec2 outerArrayRange = _GetArrayOfArraysPixel(ArrayOfArraysName, \
                                                      ArrayOfArraysName##_Size, \
                                                      ArrayOfArraysName##_TexelUvSize, \
                                                      outerArrayIndex).xy; \
        return int(outerArrayRange[1] - outerArrayRange[0]); \
    } \
    vec4 Get##ArrayOfArraysName##Element(int outerArrayIndex, int innerArrayIndex) \
    { \
        vec2 outerArrayRange = _GetArrayOfArraysPixel(ArrayOfArraysName, \
                                                      ArrayOfArraysName##_Size, \
                                                      ArrayOfArraysName##_TexelUvSize, \
                                                      outerArrayIndex).xy; \
        int elementCoord = int(outerArrayRange[0] + innerArrayIndex); \
        return _GetArrayOfArraysPixel(ArrayOfArraysName, \
                                      ArrayOfArraysName##_Size, \
                                      ArrayOfArraysName##_TexelUvSize, \
                                      elementCoord); \
    }


#endif
