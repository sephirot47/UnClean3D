#include "TexturesScene.h"

#include "Bang/Array.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Material.h"
#include "Bang/MeshRenderer.h"
#include "Bang/Texture2D.h"
#include "Bang/UIGridLayout.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILabel.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UIScrollArea.h"
#include "Bang/UIScrollBar.h"
#include "Bang/UIScrollPanel.h"
#include "Bang/UIVerticalLayout.h"

#include "EffectLayer.h"
#include "EffectLayerImplementation.h"
#include "MainScene.h"
#include "View3DScene.h"

using namespace Bang;

TexturesScene::TexturesScene()
{
    GameObjectFactory::CreateUISceneInto(this);

    UIScrollPanel *scrollPanel = GameObjectFactory::CreateUIScrollPanel();
    scrollPanel->SetForceHorizontalFit(true);

    p_gridGo = GameObjectFactory::CreateUIGameObject();
    UILayoutElement *gridLE = p_gridGo->AddComponent<UILayoutElement>();
    gridLE->SetFlexibleSize(Vector2(1.0f));
    UIGridLayout *gl = p_gridGo->AddComponent<UIGridLayout>();
    gl->SetCellSize(Vector2i(512));
    gl->SetSpacing(30);
    gl->SetPaddings(10);

    p_originalAlbedoTexCont = CreateAndAddTextureContainer("Original Albedo");
    p_originalNormalTexCont = CreateAndAddTextureContainer("Original Normal");
    p_originalRoughnessTexCont =
        CreateAndAddTextureContainer("Original Roughness");
    p_originalMetalnessTexCont =
        CreateAndAddTextureContainer("Original Metalness");

    scrollPanel->GetScrollArea()->GetBackground()->SetTint(Color::Black());
    scrollPanel->GetScrollArea()->SetContainedGameObject(p_gridGo);
    scrollPanel->GetGameObject()->SetParent(this);
}

TexturesScene::~TexturesScene()
{
}

TextureContainer *TexturesScene::CreateAndAddTextureContainer(
    const String &label)
{
    TextureContainer *textureContainer = new TextureContainer(label);
    textureContainer->SetParent(p_gridGo);
    return textureContainer;
};

void TexturesScene::Update()
{
    GameObject::Update();

    View3DScene *viewScene = MainScene::GetInstance()->GetView3DScene();

    Array<EffectLayer *> allEffectLayers = viewScene->GetAllEffectLayers();
    for (uint i = 0; i < allEffectLayers.Size(); ++i)
    {
        EffectLayer *effectLayer = allEffectLayers[i];
        if (!m_effectLayerToTexCont.ContainsKey(effectLayer))
        {
            const String effectLayerTypeName =
                effectLayer->GetImplementation()->GetTypeName();

            // Effect
            {
                String layerName =
                    "Layer " + String(i) + " (" + effectLayerTypeName + ")";
                TextureContainer *texCont =
                    CreateAndAddTextureContainer(layerName);
                m_effectLayerToTexCont.Add(effectLayer, texCont);
            }

            // Effect Mask
            {
                String layerName = "Layer " + String(i) + " Mask (" +
                                   effectLayerTypeName + ")";
                TextureContainer *maskTexCont =
                    CreateAndAddTextureContainer(layerName);
                m_effectLayerToMaskTexCont.Add(effectLayer, maskTexCont);
            }
        }
        ASSERT(m_effectLayerToTexCont.ContainsKey(effectLayer));

        TextureContainer *effectLayerTexCont =
            m_effectLayerToTexCont.Get(effectLayer);
        effectLayerTexCont->GetImageRenderer()->SetImageTexture(
            effectLayer->GetEffectTexture());

        TextureContainer *effectMaskTexCont =
            m_effectLayerToMaskTexCont.Get(effectLayer);
        effectMaskTexCont->GetImageRenderer()->SetImageTexture(
            effectLayer->GetMaskTexture());
    }

    // Remove non used effect layers
    for (auto &it : m_effectLayerToTexCont)
    {
        EffectLayer *effectLayer = it.first;
        if (!allEffectLayers.Contains(effectLayer))
        {
            TextureContainer *texCont = it.second;
            TextureContainer *texMaskCont =
                m_effectLayerToMaskTexCont.Get(it.first);
            GameObject::Destroy(texCont);
            GameObject::Destroy(texMaskCont);
            m_effectLayerToTexCont.Remove(effectLayer);
            m_effectLayerToMaskTexCont.Remove(effectLayer);
        }
    }

    if (GameObject *modelGo = viewScene->GetModelGameObject())
    {
        Array<MeshRenderer *> mrs =
            modelGo->GetComponentsInDescendantsAndThis<MeshRenderer>();
        for (MeshRenderer *mr : mrs)
        {
            if (Material *mat = mr->GetActiveMaterial())
            {
                if (Texture2D *albedoTex = mat->GetAlbedoTexture())
                {
                    p_originalAlbedoTexCont->GetImageRenderer()
                        ->SetImageTexture(albedoTex);
                }
                if (Texture2D *normalTex = mat->GetNormalMapTexture())
                {
                    p_originalNormalTexCont->GetImageRenderer()
                        ->SetImageTexture(normalTex);
                }
                if (Texture2D *roughnessTex = mat->GetRoughnessTexture())
                {
                    p_originalRoughnessTexCont->GetImageRenderer()
                        ->SetImageTexture(roughnessTex);
                }
                if (Texture2D *metalnessTex = mat->GetMetalnessTexture())
                {
                    p_originalMetalnessTexCont->GetImageRenderer()
                        ->SetImageTexture(metalnessTex);
                }
            }
        }
    }
}

void TexturesScene::OnModelChanged(Model *)
{
}

TextureContainer::TextureContainer(const String &label)
{
    GameObjectFactory::CreateUIGameObjectInto(this);

    UIVerticalLayout *vl = AddComponent<UIVerticalLayout>();
    vl->SetSpacing(5);

    UILayoutElement *goLE = AddComponent<UILayoutElement>();
    goLE->SetFlexibleSize(Vector2(1.0f));

    p_label = GameObjectFactory::CreateUILabel();
    p_label->GetText()->SetTextSize(20);
    p_label->GetText()->SetTextColor(Color::White());
    p_label->GetText()->SetContent(label);
    p_label->GetGameObject()->SetParent(this);
    UILayoutElement *textureLabelLE =
        p_label->GetGameObject()->AddComponent<UILayoutElement>();
    textureLabelLE->SetMinHeight(30);
    textureLabelLE->SetFlexibleSize(Vector2(1.0f, 0.0f));
    textureLabelLE->SetLayoutPriority(2);

    p_imageRenderer = GameObjectFactory::CreateUIImage();
    UILayoutElement *imgLE =
        p_imageRenderer->GetGameObject()->AddComponent<UILayoutElement>();
    imgLE->SetPreferredSize(Vector2i(50));
    imgLE->SetFlexibleSize(Vector2(1.0f));
    p_imageRenderer->GetGameObject()->SetParent(this);

    GameObjectFactory::AddOuterBorder(
        p_imageRenderer->GetGameObject(), Vector2i(2), Color::White());
}

TextureContainer::~TextureContainer()
{
}

UILabel *TextureContainer::GetLabel() const
{
    return p_label;
}

UIImageRenderer *TextureContainer::GetImageRenderer() const
{
    return p_imageRenderer;
}
