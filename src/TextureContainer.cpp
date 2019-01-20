#include "TextureContainer.h"

#include "Bang/GameObjectFactory.h"
#include "Bang/RectTransform.h"
#include "Bang/UIFocusable.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILabel.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UILayoutIgnorer.h"
#include "Bang/UITextRenderer.h"
#include "Bang/UIVerticalLayout.h"

using namespace Bang;

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

    p_focusable = AddComponent<UIFocusable>();

    GameObjectFactory::AddOuterBorder(
        p_imageRenderer->GetGameObject(), Vector2i(2), Color::White());
}

TextureContainer::~TextureContainer()
{
}

void TextureContainer::Update()
{
    GameObject::Update();
}

UILabel *TextureContainer::GetLabel() const
{
    return p_label;
}

UIFocusable *TextureContainer::GetFocusable() const
{
    return p_focusable;
}

UIImageRenderer *TextureContainer::GetImageRenderer() const
{
    return p_imageRenderer;
}
