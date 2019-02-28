#include "Common.glsl"

uniform vec2  MaskBrushCenter;
uniform float MaskBrushHardness;
uniform float MaskBrushStrength;
uniform float MaskBrushSize;
uniform sampler2D MaskBrushTexture;

float GetMaskBrushApportationFromCurrentMousePositionProj(vec2 posProj,
                                                          vec2 viewportSize)
{
    vec2 fragPos = posProj;
    fragPos *= viewportSize;

    vec2 relativeFragPos = (fragPos - (MaskBrushCenter - MaskBrushSize * 0.5));
    if (relativeFragPos.x > 0 &&
        relativeFragPos.y > 0 &&
        relativeFragPos.x < MaskBrushSize &&
        relativeFragPos.y < MaskBrushSize)
    {
        vec2 brushUv = relativeFragPos / MaskBrushSize;
        float brushIntensity = texture(MaskBrushTexture, brushUv).a;
        brushIntensity *= MaskBrushStrength;
        brushIntensity = clamp(brushIntensity, 0, 1);
        return brushIntensity;
    }
    return 0.0f;
}

float GetMaskBrushApportationFromCurrentMousePosition(vec3 worldPos,
                                                      mat4 projectionViewMatrix,
                                                      vec2 viewportSize)
{
    vec4 posProj = projectionViewMatrix * vec4(worldPos, 1);
    posProj.xy /= posProj.w;
    posProj.xy = posProj.xy * 0.5 + 0.5;

    return GetMaskBrushApportationFromCurrentMousePositionProj(posProj.xy,
                                                               viewportSize);
}
