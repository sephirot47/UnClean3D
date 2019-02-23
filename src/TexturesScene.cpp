#include "TexturesScene.h"

#include "Bang/Array.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Material.h"
#include "Bang/MeshRenderer.h"
#include "Bang/RectTransform.h"
#include "Bang/Texture2D.h"
#include "Bang/Transform.h"
#include "Bang/UIContentSizeFitter.h"
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
#include "TextureContainerRow.h"
#include "View3DScene.h"

using namespace Bang;

TexturesScene::TexturesScene()
{
    GameObjectFactory::CreateUISceneInto(this);

    UIScrollPanel *scrollPanel = GameObjectFactory::CreateUIScrollPanel();
    scrollPanel->SetHorizontalScrollEnabled(true);
    scrollPanel->SetVerticalScrollEnabled(true);
    scrollPanel->SetVerticalShowScrollMode(ShowScrollMode::ALWAYS);
    scrollPanel->SetHorizontalShowScrollMode(ShowScrollMode::ALWAYS);

    p_mainContainerGo = GameObjectFactory::CreateUIGameObject();
    UIVerticalLayout *vl = p_mainContainerGo->AddComponent<UIVerticalLayout>();
    vl->SetSpacing(45);
    vl->SetPaddings(30);

    p_originalRow = CreateAndAddRow();
    p_originalAlbedoTexCont =
        p_originalRow->CreateAndAddTextureContainer("Original Albedo");
    p_originalNormalTexCont =
        p_originalRow->CreateAndAddTextureContainer("Original Normal");
    p_originalRoughnessTexCont =
        p_originalRow->CreateAndAddTextureContainer("Original Roughness");
    p_originalMetalnessTexCont =
        p_originalRow->CreateAndAddTextureContainer("Original Metalness");

    p_finalRow = CreateAndAddRow();
    p_finalAlbedoTexCont =
        p_finalRow->CreateAndAddTextureContainer("Final Albedo");
    p_finalNormalTexCont =
        p_finalRow->CreateAndAddTextureContainer("Final Normal");
    p_finalRoughnessTexCont =
        p_finalRow->CreateAndAddTextureContainer("Final Roughness");
    p_finalMetalnessTexCont =
        p_finalRow->CreateAndAddTextureContainer("Final Metalness");

    auto csf = p_mainContainerGo->AddComponent<UIContentSizeFitter>();
    csf->SetVerticalSizeType(LayoutSizeType::PREFERRED);
    csf->SetHorizontalSizeType(LayoutSizeType::PREFERRED);
    p_mainContainerGo->GetRectTransform()->SetPivotPosition(Vector2(-1, 1));

    scrollPanel->GetScrollArea()->GetBackground()->SetTint(Color::Black());
    scrollPanel->GetScrollArea()->SetContainedGameObject(p_mainContainerGo);
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

void TexturesScene::Update()
{
    GameObject::Update();

    View3DScene *view3DScene = MainScene::GetInstance()->GetView3DScene();

    // Clean or add effect layer texture containers as needed
    Array<EffectLayer *> allEffectLayers = view3DScene->GetAllEffectLayers();
    for (uint i = 0; i < allEffectLayers.Size(); ++i)
    {
        EffectLayer *effectLayer = allEffectLayers[i];
        if (!p_effectLayerToRow.ContainsKey(effectLayer))
        {
            // Effect
            {
                TextureContainerRow *texContRow = CreateAndAddRow(effectLayer);

                TextureContainer *texCont =
                    texContRow->CreateAndAddEffectColorTextureContainer(
                        effectLayer);
                texCont->GetImageRenderer()->SetImageTexture(
                    effectLayer->GetEffectColorTexture());

                TextureContainer *mergedMaskTexCont =
                    texContRow->CreateAndAddMergedMaskTextureContainer(
                        effectLayer);
                mergedMaskTexCont->GetImageRenderer()->SetImageTexture(
                    effectLayer->GetMergedMaskTexture());

                p_effectLayerToRow.Add(effectLayer, texContRow);
            }
        }
        ASSERT(p_effectLayerToRow.ContainsKey(effectLayer));

        // Effect Masks
        TextureContainerRow *texContRow = p_effectLayerToRow.Get(effectLayer);
        for (EffectLayerMask *effectLayerMask : effectLayer->GetMasks())
        {
            if (!texContRow->GetMaskTextureContainersMap().ContainsKey(
                    effectLayerMask))
            {
                texContRow->CreateAndAddMaskTextureContainer(effectLayerMask);
            }
        }
    }

    // Remove non used texture containers
    const auto effectLayersOfTextureContGroups = p_effectLayerToRow.GetKeys();
    for (EffectLayer *effectLayer : effectLayersOfTextureContGroups)
    {
        TextureContainerRow *texContRow = p_effectLayerToRow.Get(effectLayer);
        if (!allEffectLayers.Contains(effectLayer))
        {
            texContRow->RemoveTextureContainer(
                texContRow->GetEffectColorTextureContainer());
            texContRow->RemoveTextureContainer(
                texContRow->GetMergedMaskEffectTextureContainer());
            for (auto &it : texContRow->GetMaskTextureContainersMap())
            {
                texContRow->RemoveTextureContainer(it.second);
            }
        }
        else
        {
            // Remove non used texture container masks
            Array<EffectLayerMask *> masksWithTextureContainers =
                texContRow->GetMaskTextureContainersMap().GetKeys();
            for (EffectLayerMask *effectLayerMask : masksWithTextureContainers)
            {
                if (!effectLayer->GetMasks().Contains(effectLayerMask))
                {
                    TextureContainer *maskTexCont =
                        texContRow->GetMaskTextureContainersMap().Get(
                            effectLayerMask);
                    texContRow->RemoveTextureContainer(maskTexCont);
                }
            }
        }
    }

    // Remove rows if needed
    Array<TextureContainerRow *> texContRows = p_effectLayerToRow.GetValues();
    for (TextureContainerRow *texContRow : texContRows)
    {
        if (texContRow->CanBeDestroyed())
        {
            GameObject::Destroy(texContRow);
            p_effectLayerToRow.RemoveValues(texContRow);
        }
    }

    // Update texture containers
    for (auto &itEffectLayer : p_effectLayerToRow)
    {
        EffectLayer *effectLayer = itEffectLayer.first;
        TextureContainerRow *texContRow = itEffectLayer.second;
        ASSERT(allEffectLayers.Contains(effectLayer));

        TextureContainer *effectTexCont =
            texContRow->GetEffectColorTextureContainer();
        TextureContainer *mergedMaskTexCont =
            texContRow->GetMergedMaskEffectTextureContainer();
        effectTexCont->SetLabel(effectLayer->GetName());
        mergedMaskTexCont->SetLabel(effectLayer->GetName() + "_MergedMask");
        effectTexCont->GetImageRenderer()->SetImageTexture(
            effectLayer->GetEffectColorTexture());
        mergedMaskTexCont->GetImageRenderer()->SetImageTexture(
            effectLayer->GetMergedMaskTexture());

        for (auto &itMask : texContRow->GetMaskTextureContainersMap())
        {
            EffectLayerMask *effectLayerMask = itMask.first;
            TextureContainer *maskTexCont = itMask.second;
            ASSERT(effectLayer->GetMasks().Contains(effectLayerMask));

            maskTexCont->SetLabel(effectLayerMask->GetName());
            maskTexCont->GetImageRenderer()->SetImageTexture(
                effectLayerMask->GetMaskTexture());
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
        p_bigImageGo->SetEnabled(false);
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

TextureContainerRow *TexturesScene::CreateAndAddRow(EffectLayer *effectLayer)
{
    TextureContainerRow *row = new TextureContainerRow(effectLayer);
    row->SetParent(p_mainContainerGo);
    return row;
}

void TexturesScene::OnModelChanged(Model *)
{
}
