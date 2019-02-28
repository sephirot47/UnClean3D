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

    p_label = GameObjectFactory::CreateUILabel();
    p_label->GetText()->SetTextSize(20);
    p_label->GetText()->SetTextColor(Color::White());
    SetLabel(label);
    p_label->GetGameObject()->SetParent(this);

    UILayoutElement *textureLabelLE =
        p_label->GetGameObject()->AddComponent<UILayoutElement>();
    textureLabelLE->SetMinHeight(30);
    textureLabelLE->SetFlexibleSize(Vector2(1.0f, 0.0f));
    textureLabelLE->SetLayoutPriority(2);

    p_imageRenderer = GameObjectFactory::CreateUIImage();
    p_imageLE =
        p_imageRenderer->GetGameObject()->AddComponent<UILayoutElement>();
    p_imageLE->SetMinSize(Vector2i(300));
    p_imageLE->SetFlexibleSize(Vector2(1.0f));
    p_imageRenderer->GetGameObject()->SetParent(this);

    p_border = GameObjectFactory::AddOuterBorder(
        p_imageRenderer->GetGameObject(), Vector2i(2), Color::Black());

    p_focusable = AddComponent<UIFocusable>();
    p_focusable->AddEventCallback([this](UIFocusable *, const UIEvent &event) {
        if (m_canBeFocused)
        {
            switch (event.type)
            {
                case UIEvent::Type::MOUSE_ENTER:
                    p_border->SetTint(Color::Orange());
                    break;

                case UIEvent::Type::MOUSE_EXIT:
                    p_border->SetTint(Color::Black());
                    break;

                default: break;
            }
            return UIEventResult::IGNORE;
        }
    });
}

TextureContainer::~TextureContainer()
{
}

void TextureContainer::Update()
{
    GameObject::Update();
}

void TextureContainer::SetLabel(const String &label)
{
    String ellidedLabel = label.ElideRight(20);
    GetLabel()->GetText()->SetContent(ellidedLabel);
}

void TextureContainer::SetCanBeFocused(bool canBeFocused)
{
    m_canBeFocused = canBeFocused;
    if (!canBeFocused)
    {
        p_border->SetTint(Color::White());
    }
    else
    {
        GetFocusable()->SetCursorType(Cursor::Type::HAND);
        p_border->SetTint(Color::Black());
    }
}

UILabel *TextureContainer::GetLabel() const
{
    return p_label;
}

UIImageRenderer *TextureContainer::GetBorder() const
{
    return p_border;
}

UIFocusable *TextureContainer::GetFocusable() const
{
    return p_focusable;
}

UIImageRenderer *TextureContainer::GetImageRenderer() const
{
    return p_imageRenderer;
}

UILayoutElement *TextureContainer::GetImageLayoutElement() const
{
    return p_imageLE;
}
