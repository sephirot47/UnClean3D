#ifndef EFFECTLAYERMASK_H
#define EFFECTLAYERMASK_H

#include "Bang/Bang.h"
#include "Bang/Texture2D.h"
#include "BangEditor/BangEditor.h"

using namespace Bang;

class EffectLayerMask
{
public:
    EffectLayerMask();
    virtual ~EffectLayerMask();

private:
    AH<Texture2D> m_maskTexture;
};

#endif  // EFFECTLAYERMASK_H
