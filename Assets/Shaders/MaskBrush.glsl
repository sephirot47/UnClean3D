#include "Common.glsl"

uniform bool  MaskBrushEnabled;
uniform vec2  MaskBrushCenter;
uniform float MaskBrushHardness;
uniform float MaskBrushStrength;
uniform float MaskBrushSize;

float GetMaskBrushApportationFromCurrentMousePositionProj(vec2 posProj,
                                                          vec2 viewportSize)
{
    if (MaskBrushEnabled)
    {
        vec2 fragPos = posProj;
        fragPos *= viewportSize;

        float dist = distance(MaskBrushCenter, fragPos);
        if (dist < MaskBrushSize)
        {
            float distNorm = (dist / MaskBrushSize);
            float halfHardness = (MaskBrushHardness * 0.5f);
            float hh = halfHardness;
            float brushIntensity = smoothstep(hh, 1 - hh, (1.0 - distNorm + hh));
            brushIntensity *= MaskBrushStrength;
            return brushIntensity;
        }
    }
    return 0.0f;
}

float GetMaskBrushApportationFromCurrentMousePosition(vec3 worldPos,
                                                      mat4 projectionViewMatrix,
                                                      vec2 viewportSize)
{
    if (MaskBrushEnabled)
    {
        vec4 posProj = projectionViewMatrix * vec4(worldPos, 1);
        posProj.xy /= posProj.w;
        posProj.xy = posProj.xy * 0.5 + 0.5;

        return GetMaskBrushApportationFromCurrentMousePositionProj(posProj.xy,
                                                                   viewportSize);
    }
    return 0.0f;
}
