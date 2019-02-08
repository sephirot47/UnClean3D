

vec4 GetArrayOfArraysData(sampler2D arrayOfArrays, int index)
{
    ivec2 cellTexCoordTriangleListBeginEnd = ivec2(GetTextureData(GridTexture, cellCoord).xy);

    vec2 textureSize = vec2(textureSize(dataTexture, 0));
    vec2 texelSize = (1.0 / textureSize);
    vec2 texCoord = vec2(mod(index, textureSize.x), index / textureSize.x);
    vec2 uvs = texCoord * texelSize;
    return texture(dataTexture, uvs, 0);
}
