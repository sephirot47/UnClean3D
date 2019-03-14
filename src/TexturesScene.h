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
class TextureContainerRow;
class EffectLayerMask;
class TexturesScene : public Scene
{
public:
    TexturesScene();
    virtual ~TexturesScene() override;

    // Scene
    void Update() override;

    TextureContainerRow *CreateAndAddRow(EffectLayer *effectLayer = nullptr);
    void OnModelChanged(Model *model);

private:
    GameObject *p_mainContainerGo = nullptr;
    GameObject *p_bigImageGo = nullptr;
    UIImageRenderer *p_bigImageRenderer = nullptr;
    TextureContainer *p_originalAlbedoTexCont = nullptr;
    TextureContainer *p_originalNormalTexCont = nullptr;
    TextureContainer *p_originalRoughnessTexCont = nullptr;
    TextureContainer *p_originalMetalnessTexCont = nullptr;
    TextureContainer *p_finalAlbedoTexCont = nullptr;
    TextureContainer *p_finalNormalTexCont = nullptr;
    TextureContainer *p_finalHeightfieldTexCont = nullptr;
    TextureContainer *p_finalRoughnessTexCont = nullptr;
    TextureContainer *p_finalMetalnessTexCont = nullptr;

    TextureContainerRow *p_originalRow = nullptr;
    TextureContainerRow *p_finalRow = nullptr;
    Map<EffectLayer *, TextureContainerRow *> p_effectLayerToRow;
};

#endif  // TEXTURESSCENE_H
