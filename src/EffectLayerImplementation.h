#ifndef EFFECTLAYERIMPLEMENTATION_H
#define EFFECTLAYERIMPLEMENTATION_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayer.h"

using namespace Bang;

class EffectLayerImplementation
{
public:
    EffectLayerImplementation();
    virtual ~EffectLayerImplementation();

    const EffectLayerParameters &GetParameters() const;
    virtual EffectLayer::Type GetEffectLayerType() const = 0;
    EffectLayer *GetEffectLayer() const;
    virtual String GetTypeName() const = 0;

protected:
    virtual Path GetGenerateEffectTextureShaderProgramPath() const = 0;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp);

private:
    EffectLayer *p_effectLayer = nullptr;

    friend class EffectLayer;
};

#endif  // EFFECTLAYERIMPLEMENTATION_H