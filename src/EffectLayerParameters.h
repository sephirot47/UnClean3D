#ifndef EFFECTLAYERPARAMETERS_H
#define EFFECTLAYERPARAMETERS_H

#include "Bang/Bang.h"
#include "Bang/Color.h"

using namespace Bang;

struct EffectLayerParameters
{
    float dirtSeed = 0.0f;
    float dirtFrequency = 4.0f;
    float dirtFrequencyMultiply = 2.5f;
    float dirtAmplitude = 1.0f;
    float dirtAmplitudeMultiply = 0.6f;
    Color dirtColor0 = Color::Red();
    Color dirtColor1 = Color::Black();
};

#endif  // EFFECTLAYERPARAMETERS_H
