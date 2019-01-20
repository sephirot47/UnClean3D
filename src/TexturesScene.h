#ifndef TEXTURESSCENE_H
#define TEXTURESSCENE_H

#include "Bang/Bang.h"
#include "Bang/Model.h"
#include "Bang/Scene.h"
#include "Bang/UIImageRenderer.h"
#include "BangEditor/BangEditor.h"

using namespace Bang;
namespace Bang
{
class UILabel;
};

class EffectLayer;
class TextureContainer;
class TexturesScene : public Scene
{
public:
    TexturesScene();
    virtual ~TexturesScene() override;

    // Scene
    void Update() override;

    void OnModelChanged(Model *model);

private:
    GameObject *p_gridGo = nullptr;
    GameObject *p_bigImageGo = nullptr;
    UIImageRenderer *p_bigImageRenderer = nullptr;
    TextureContainer *p_originalAlbedoTexCont = nullptr;
    TextureContainer *p_originalNormalTexCont = nullptr;
    TextureContainer *p_originalRoughnessTexCont = nullptr;
    TextureContainer *p_originalMetalnessTexCont = nullptr;
    TextureContainer *p_finalAlbedoTexCont = nullptr;
    TextureContainer *p_finalNormalTexCont = nullptr;
    TextureContainer *p_finalRoughnessTexCont = nullptr;
    TextureContainer *p_finalMetalnessTexCont = nullptr;
    Map<EffectLayer *, TextureContainer *> m_effectLayerToTexCont;
    Map<EffectLayer *, TextureContainer *> m_effectLayerToMaskTexCont;

    TextureContainer *CreateAndAddTextureContainer(const String &label);
};

#endif  // TEXTURESSCENE_H
