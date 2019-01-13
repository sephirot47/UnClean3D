#ifndef EFFECTLAYERCOMPOSITER_H
#define EFFECTLAYERCOMPOSITER_H

#include "Bang/Array.h"
#include "Bang/AssetHandle.h"
#include "Bang/ShaderProgram.h"
#include "Bang/Texture2D.h"
#include "BangEditor/BangEditor.h"

namespace Bang
{
class Texture2D;
class Framebuffer;
}

class EffectLayer;
using namespace Bang;

class EffectLayerCompositer
{
public:
    EffectLayerCompositer();
    virtual ~EffectLayerCompositer();

    void ReloadShaders();
    Texture2D *CompositeLayers(Texture2D *albedoReadTexture,
                               const Array<EffectLayer *> &effectLayers);

private:
    Framebuffer *m_framebuffer = nullptr;
    AH<Texture2D> m_auxiliarTexture;
    AH<ShaderProgram> m_compositeLayersSP;

    AH<Texture2D> m_compositeTexture;
};

#endif  // EFFECTLAYERCOMPOSITER_H
