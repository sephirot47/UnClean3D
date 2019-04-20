#ifndef EFFECTLAYERMASKIMPLEMENTATIONEDGES_H
#define EFFECTLAYERMASKIMPLEMENTATIONEDGES_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "GLSLArrayOfArrays.h"
#include "EffectLayerMaskImplementationGPU.h"

namespace Bang
{
class VBO;
};

using namespace Bang;

class EffectLayerMaskImplementationEdges
    : public EffectLayerMaskImplementationGPU
{
public:
    SERIALIZABLE(EffectLayerMaskImplementationEdges)

    EffectLayerMaskImplementationEdges();
    virtual ~EffectLayerMaskImplementationEdges() override;

    // Serializable
    virtual void Reflect() override;

protected:
    // EffectLayerMaskImplementation
    virtual EffectLayerMask::Type GetEffectLayerMaskType() const override;
    virtual String GetTypeName() const override;
    virtual Path GetGenerateEffectTextureShaderProgramPath() const override;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp,
                                           MeshRenderer *meshRend) override;

private:
    AH<Texture2D> m_curvaturesTexture;
    bool m_curvaturesCreated = false;
    GLSLArrayOfArrays m_curvaturesGLSLArray;
    // VBO *m_curvaturesVBO = nullptr;

    float m_threshold = 1.0f;
};

#endif  // EFFECTLAYERMASKIMPLEMENTATIONEDGES_H
