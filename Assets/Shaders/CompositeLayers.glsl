const int EFFECT_LAYER_BLEND_MODE_ADD      = 0;
const int EFFECT_LAYER_BLEND_MODE_INV_ADD  = 1;
const int EFFECT_LAYER_BLEND_MODE_MULT     = 2;
const int EFFECT_LAYER_BLEND_MODE_INV_MULT = 3;

uniform sampler2D EffectLayerTexture_0;
uniform sampler2D EffectLayerTexture_1;
uniform sampler2D EffectLayerTexture_2;
uniform sampler2D EffectLayerTexture_3;
uniform sampler2D EffectLayerTexture_4;
uniform sampler2D EffectLayerTexture_5;
uniform sampler2D EffectLayerTexture_6;
uniform sampler2D EffectLayerTexture_7;
uniform sampler2D EffectLayerTexture_8;
uniform sampler2D EffectLayerTexture_9;
uniform bool EffectLayerVisibles[10];
uniform int EffectLayerBlendModes[10];
uniform vec4 EffectLayerTints[10];
uniform int NumEffectLayers;

void CompositeLayers(in vec2 uv,
                     in vec3 albedoColor,
                     out vec3 resultAlbedoColor)
{
    #define SAMPLE_EFFECT_LAYER(k) texture2D(EffectLayerTexture_##k, uv).rgb;

    resultAlbedoColor = albedoColor;

    // Apply effect layers
    for (int i = 0; i < NumEffectLayers; ++i)
    {
        if (!EffectLayerVisibles[i])
        {
            continue;
        }

        vec3 effectLayerColor;
        switch (i)
        {
            case 0: effectLayerColor = SAMPLE_EFFECT_LAYER(0); break;
            case 1: effectLayerColor = SAMPLE_EFFECT_LAYER(1); break;
            case 2: effectLayerColor = SAMPLE_EFFECT_LAYER(2); break;
            case 3: effectLayerColor = SAMPLE_EFFECT_LAYER(3); break;
            case 4: effectLayerColor = SAMPLE_EFFECT_LAYER(4); break;
            case 5: effectLayerColor = SAMPLE_EFFECT_LAYER(5); break;
            case 6: effectLayerColor = SAMPLE_EFFECT_LAYER(6); break;
            case 7: effectLayerColor = SAMPLE_EFFECT_LAYER(7); break;
            case 8: effectLayerColor = SAMPLE_EFFECT_LAYER(8); break;
            case 9: effectLayerColor = SAMPLE_EFFECT_LAYER(9); break;
        }

        vec3 tint = EffectLayerTints[i].rgb;
        int effectLayerBlendMode = EffectLayerBlendModes[i];
        switch (effectLayerBlendMode)
        {
            case EFFECT_LAYER_BLEND_MODE_ADD:
            resultAlbedoColor.rgb += (effectLayerColor * tint);
            break;
            case EFFECT_LAYER_BLEND_MODE_INV_ADD:
            resultAlbedoColor.rgb += ((vec3(1) - effectLayerColor * (vec3(1)-tint)));
            break;
            case EFFECT_LAYER_BLEND_MODE_MULT:
            resultAlbedoColor.rgb *= (effectLayerColor * tint);
            break;
            case EFFECT_LAYER_BLEND_MODE_INV_MULT:
            resultAlbedoColor.rgb *= ((vec3(1) - effectLayerColor * (vec3(1)-tint)));
            break;
        }
    }
}

