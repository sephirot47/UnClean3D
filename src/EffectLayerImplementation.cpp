#include "EffectLayerImplementation.h"

#include "Bang/GameObject.h"
#include "Bang/Transform.h"

#include "MainScene.h"
#include "View3DScene.h"

using namespace Bang;

EffectLayerImplementation::EffectLayerImplementation()
{
}

EffectLayerImplementation::~EffectLayerImplementation()
{
}

const EffectLayerParameters &EffectLayerImplementation::GetParameters() const
{
    return GetEffectLayer()->GetParameters();
}

void EffectLayerImplementation::SetGenerateEffectUniforms(ShaderProgram *sp)
{
    View3DScene *view3DScene = MainScene::GetInstance()->GetView3DScene();
    sp->SetMatrix4("SceneModelMatrix",
                   view3DScene->GetModelGameObject()
                       ->GetTransform()
                       ->GetLocalToWorldMatrix());
}

EffectLayer *EffectLayerImplementation::GetEffectLayer() const
{
    return p_effectLayer;
}
