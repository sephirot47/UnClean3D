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
                         Texture2D *metalnessOriginalTexture);

    Texture2D *GetFinalAlbedoTexture() const;
    Texture2D *GetFinalNormalTexture() const;
    Texture2D *GetFinalHeightTexture() const;
    Texture2D *GetFinalRoughnessTexture() const;
    Texture2D *GetFinalMetalnessTexture() const;

private:
    Framebuffer *m_framebuffer = nullptr;

    AH<Texture2D> m_albedoPingPongTexture0;
    AH<Texture2D> m_albedoPingPongTexture1;
    AH<Texture2D> m_normalPingPongTexture0;
    AH<Texture2D> m_normalPingPongTexture1;
    AH<Texture2D> m_heightPingPongTexture0;
    AH<Texture2D> m_heightPingPongTexture1;
    AH<Texture2D> m_roughnessPingPongTexture0;
    AH<Texture2D> m_roughnessPingPongTexture1;
    AH<Texture2D> m_metalnessPingPongTexture0;
    AH<Texture2D> m_metalnessPingPongTexture1;

    AH<Texture2D> p_finalAlbedoTexture;
    AH<Texture2D> p_finalNormalTexture;
    AH<Texture2D> p_finalHeightTexture;
    AH<Texture2D> p_finalRoughnessTexture;
    AH<Texture2D> p_finalMetalnessTexture;

    AH<ShaderProgram> m_compositeLayersSP;
    AH<ShaderProgram> m_heightfieldToNormalTextureSP;
};

#endif  // EFFECTLAYERCOMPOSITER_H
