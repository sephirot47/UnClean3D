#include "TextureContainerRow.h"

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

TextureContainerRow::TextureContainerRow(EffectLayer *effectLayer)
{
    GameObjectFactory::CreateUIGameObjectInto(this);

    p_effectLayer = effectLayer;
    auto hl = AddComponent<UIHorizontalLayout>();
    hl->SetSpacing(20);
}

TextureContainerRow::~TextureContainerRow()
{
}

TextureContainer *TextureContainerRow::CreateAndAddTextureContainer(
    const String &name)
{
    TextureContainer *texCont = new TextureContainer(name);
    texCont->SetParent(this);
    return texCont;
}

TextureContainer *TextureContainerRow::CreateAndAddEffectColorTextureContainer(
    EffectLayer *effectLayer,
    const String &name)
{
    p_effectColorTextureContainer = CreateAndAddTextureContainer(name);
    p_effectColorTextureContainer->GetImageRenderer()->SetImageTexture(
        effectLayer->GetEffectColorTexture());
    return p_effectColorTextureContainer;
}

TextureContainer *TextureContainerRow::CreateAndAddMergedMaskTextureContainer(
    EffectLayer *effectLayer,
    const String &name)
{
    p_mergedMaskEffectTextureContainer = CreateAndAddTextureContainer(name);
    p_mergedMaskEffectTextureContainer->GetImageRenderer()->SetImageTexture(
        effectLayer->GetMergedMaskTexture());
    return p_mergedMaskEffectTextureContainer;
}

TextureContainer *TextureContainerRow::CreateAndAddMaskTextureContainer(
    EffectLayerMask *effectLayerMask,
    const String &name)
{
    TextureContainer *textureContainer = CreateAndAddTextureContainer(name);
    textureContainer->GetImageRenderer()->SetImageTexture(
        effectLayerMask->GetMaskTexture());
    p_maskTextureContainers.Add(effectLayerMask, textureContainer);
    return textureContainer;
}

void TextureContainerRow::RemoveTextureContainer(
    TextureContainer *textureContainer)
{
    GameObject::Destroy(textureContainer);

    if (p_effectColorTextureContainer == textureContainer)
    {
        p_effectColorTextureContainer = nullptr;
    }
    else if (p_mergedMaskEffectTextureContainer == textureContainer)
    {
        p_mergedMaskEffectTextureContainer = nullptr;
    }
    else
    {
        p_maskTextureContainers.RemoveValues(textureContainer);
    }
}

bool TextureContainerRow::CanBeDestroyed() const
{
    return !GetEffectColorTextureContainer() &&
           !GetMergedMaskEffectTextureContainer() &&
           GetMaskTextureContainersMap().IsEmpty();
}

EffectLayer *TextureContainerRow::GetEffectLayer() const
{
    return p_effectLayer;
}

TextureContainer *TextureContainerRow::GetEffectColorTextureContainer() const
{
    return p_effectColorTextureContainer;
}

TextureContainer *TextureContainerRow::GetMergedMaskEffectTextureContainer()
    const
{
    return p_mergedMaskEffectTextureContainer;
}

const Map<EffectLayerMask *, TextureContainer *>
    &TextureContainerRow::GetMaskTextureContainersMap() const
{
    return p_maskTextureContainers;
}
