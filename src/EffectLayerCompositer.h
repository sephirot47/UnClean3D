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
    void CompositeLayers(const Array<EffectLayer *> &effectLayers,
                         Texture2D *albedoOriginalTexture,
                         Texture2D *normalOriginalTexture,
                         Texture2D *roughnessOriginalTexture,
                         Texture2D *metalnessOriginalTexture,
                         Texture2D **outAlbedoTexture,
                         Texture2D **outNormalTexture,
                         Texture2D **outRoughnessTexture,
                         Texture2D **outMetalnessTexture);

private:
    Framebuffer *m_framebuffer = nullptr;

    AH<Texture2D> m_albedoPingPongTexture0;
    AH<Texture2D> m_albedoPingPongTexture1;
    AH<Texture2D> m_normalPingPongTexture0;
    AH<Texture2D> m_normalPingPongTexture1;
    AH<Texture2D> m_roughnessPingPongTexture0;
    AH<Texture2D> m_roughnessPingPongTexture1;
    AH<Texture2D> m_metalnessPingPongTexture0;
    AH<Texture2D> m_metalnessPingPongTexture1;

    AH<ShaderProgram> m_compositeLayersSP;
};

#endif  // EFFECTLAYERCOMPOSITER_H
