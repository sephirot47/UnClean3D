#include "TexturesScene.h"

#include "Bang/Array.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Material.h"
#include "Bang/MeshRenderer.h"
#include "Bang/RectTransform.h"
#include "Bang/Texture2D.h"
#include "Bang/Transform.h"
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
#include "EffectLayerCompositer.h"
#include "EffectLayerMaskImplementation.h"
#include "MainScene.h"
#include "TextureContainer.h"
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
    gl->SetCellSize(Vector2i(200));
    gl->SetSpacing(30);
    gl->SetPaddings(10);

    p_originalAlbedoTexCont = CreateAndAddTextureContainer("Original Albedo");
    p_originalNormalTexCont = CreateAndAddTextureContainer("Original Normal");
    p_originalRoughnessTexCont =
        CreateAndAddTextureContainer("Original Roughness");
    p_originalMetalnessTexCont =
        CreateAndAddTextureContainer("Original Metalness");

    p_finalAlbedoTexCont = CreateAndAddTextureContainer("Final Albedo");
    p_finalNormalTexCont = CreateAndAddTextureContainer("Final Normal");
    p_finalRoughnessTexCont = CreateAndAddTextureContainer("Final Roughness");
    p_finalMetalnessTexCont = CreateAndAddTextureContainer("Final Metalness");

    scrollPanel->GetScrollArea()->GetBackground()->SetTint(Color::Black());
    scrollPanel->GetScrollArea()->SetContainedGameObject(p_gridGo);
    scrollPanel->GetGameObject()->SetParent(this);

    p_bigImageGo = GameObjectFactory::CreateUIGameObject();
    UIImageRenderer *bigImageBg = p_bigImageGo->AddComponent<UIImageRenderer>();
    bigImageBg->SetTint(Color::Black());
    p_bigImageRenderer = p_bigImageGo->AddComponent<UIImageRenderer>();
    p_bigImageRenderer->SetEnabled(true);
    p_bigImageGo->GetRectTransform()->SetMargins(100);
    p_bigImageGo->GetTransform()->TranslateLocal(Vector3(0, 0, -0.5));
    GameObjectFactory::AddOuterBorder(
        p_bigImageGo, Vector2i(5), Color::Black());
    p_bigImageGo->SetParent(this);
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

    View3DScene *view3DScene = MainScene::GetInstance()->GetView3DScene();

    // Clean or add effect layer texture containers as needed
    Array<EffectLayer *> allEffectLayers = view3DScene->GetAllEffectLayers();
    for (uint i = 0; i < allEffectLayers.Size(); ++i)
    {
        EffectLayer *effectLayer = allEffectLayers[i];
        if (!m_effectLayerToTexCont.ContainsKey(effectLayer))
        {
            // Effect
            {
                String layerName = effectLayer->GetName();
                TextureContainer *texCont =
                    CreateAndAddTextureContainer(layerName);
                m_effectLayerToTexCont.Add(effectLayer, texCont);
            }

            // Effect Masks
            for (EffectLayerMask *effectLayerMask : effectLayer->GetMasks())
            {
                String layerName = effectLayerMask->GetName();
                TextureContainer *maskTexCont =
                    CreateAndAddTextureContainer(layerName);
                m_effectLayerToMaskTexCont.Add(effectLayer, maskTexCont);
            }
        }
        ASSERT(m_effectLayerToTexCont.ContainsKey(effectLayer));

        TextureContainer *effectLayerTexCont =
            m_effectLayerToTexCont.Get(effectLayer);
        effectLayerTexCont->GetImageRenderer()->SetImageTexture(
            effectLayer->GetEffectColorTexture());

        TextureContainer *effectMaskTexCont =
            m_effectLayerToMaskTexCont.Get(effectLayer);
        effectMaskTexCont->GetImageRenderer()->SetImageTexture(
            effectLayer->GetMergedMaskTexture());
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

    // Set big image texture if there is some overed texture
    {
        TextureContainer *overedTexCont = nullptr;
        Array<TextureContainer *> allTextureContainers =
            GetObjectsInDescendants<TextureContainer>();
        for (TextureContainer *texCont : allTextureContainers)
        {
            if (texCont->GetFocusable()->IsMouseOver())
            {
                overedTexCont = texCont;
                break;
            }
        }

        if (overedTexCont)
        {
            p_bigImageRenderer->SetImageTexture(
                overedTexCont->GetImageRenderer()->GetImageTexture());
        }
        p_bigImageGo->SetEnabled(overedTexCont != nullptr);
    }

    if (GameObject *modelGo = view3DScene->GetModelGameObject())
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

        EffectLayerCompositer *compositer =
            view3DScene->GetEffectLayerCompositer();
        p_finalAlbedoTexCont->GetImageRenderer()->SetImageTexture(
            compositer->GetFinalAlbedoTexture());
        p_finalNormalTexCont->GetImageRenderer()->SetImageTexture(
            compositer->GetFinalNormalTexture());
        p_finalRoughnessTexCont->GetImageRenderer()->SetImageTexture(
            compositer->GetFinalRoughnessTexture());
        p_finalMetalnessTexCont->GetImageRenderer()->SetImageTexture(
            compositer->GetFinalMetalnessTexture());
    }
}

void TexturesScene::OnModelChanged(Model *)
{
}
