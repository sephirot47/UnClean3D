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
    TextureContainer *p_originalAlbedoTexCont = nullptr;
    TextureContainer *p_originalNormalTexCont = nullptr;
    TextureContainer *p_originalRoughnessTexCont = nullptr;
    TextureContainer *p_originalMetalnessTexCont = nullptr;
    Map<EffectLayer *, TextureContainer *> m_effectLayerToTexCont;
    Map<EffectLayer *, TextureContainer *> m_effectLayerToMaskTexCont;

    TextureContainer *CreateAndAddTextureContainer(const String &label);
};

class TextureContainer : public GameObject
{
public:
    TextureContainer(const String &label = "Label");
    virtual ~TextureContainer() override;

    UILabel *GetLabel() const;
    UIImageRenderer *GetImageRenderer() const;

private:
    UIImageRenderer *p_imageRenderer = nullptr;
    UILabel *p_label = nullptr;
};

#endif  // TEXTURESSCENE_H
