#include "UIEffectLayerRow.h"

#include "Bang/AARect.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Input.h"
#include "Bang/Random.h"
#include "Bang/RectTransform.h"
#include "Bang/UIButton.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILabel.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UITextRenderer.h"
#include "Bang/UITheme.h"
#include "Bang/UIToolButton.h"
#include "Bang/UIVerticalLayout.h"

#include "ControlPanel.h"
#include "MainScene.h"
#include "UIEffectLayers.h"
#include "View3DScene.h"

using namespace Bang;

UIEffectLayerRow::UIEffectLayerRow(UIEffectLayers *uiEffectLayers)
{
    GameObjectFactory::CreateUIGameObjectInto(this);

    p_uiEffectLayers = uiEffectLayers;

    p_focusable = AddComponent<UIFocusable>();
    p_focusable->AddEventCallback([this](UIFocusable *, const UIEvent &event) {
        switch (event.type)
        {
            case UIEvent::Type::MOUSE_CLICK_FULL:
                p_uiEffectLayers->SetSelection(this);
                return UIEventResult::INTERCEPT;
            default: break;
        }
        return UIEventResult::IGNORE;
    });

    UIHorizontalLayout *hl = AddComponent<UIHorizontalLayout>();
    hl->SetSpacing(5);
    hl->SetPaddings(5);
    hl->SetPaddingRight(10);
    hl->SetPaddingLeft(10);

    p_bg = AddComponent<UIImageRenderer>();
    // p_bg->SetTint(Color::White().WithValue(0.95f));
    GameObjectFactory::AddInnerBorder(p_bg->GetGameObject());

    UILayoutElement *le = AddComponent<UILayoutElement>();
    le->SetFlexibleWidth(1.0f);
    le->SetMinHeight(40);

    p_layerNameLabel = GameObjectFactory::CreateUILabel();
    static int xxx = 0;
    xxx += 1;
    p_layerNameLabel->GetText()->SetContent("Layer row " + String(xxx));
    p_layerNameLabel->GetText()->SetHorizontalAlign(HorizontalAlignment::LEFT);
    p_layerNameLabel->GetGameObject()->SetParent(this);

    GameObjectFactory::CreateUIHSpacer(LayoutSizeType::FLEXIBLE, 1.0f)
        ->SetParent(this);

    p_visibleButton = GameObjectFactory::CreateUIToolButton("V");
    p_visibleButton->SetOn(true);
    p_visibleButton->GetGameObject()->SetParent(this);

    UIButton *p_removeLayerButton = GameObjectFactory::CreateUIButton("-");
    p_removeLayerButton->AddClickedCallback([this]() {
        ControlPanel *cp = MainScene::GetInstance()->GetControlPanel();
        uint idx = p_uiEffectLayers->GetUIEffectLayerRows().IndexOf(this);
        cp->RemoveEffectLayer(idx);
    });
    p_removeLayerButton->GetGameObject()->SetParent(this);
}

UIEffectLayerRow::~UIEffectLayerRow()
{
}

void UIEffectLayerRow::Update()
{
    GameObject::Update();

    Color bgColor;
    if (IsSelected())
    {
        bgColor = UITheme::GetSelectedColor();
    }
    else
    {
        if (GetRectTransform()->GetViewportAARect().Contains(
                Vector2(Input::GetMousePosition())))
        {
            bgColor = UITheme::GetOverColor();
        }
        else
        {
            bgColor = Color::White().WithValue(0.95f);
        }
    }
    p_bg->SetTint(bgColor);
}

bool UIEffectLayerRow::IsSelected() const
{
    return p_uiEffectLayers->GetSelectedEffectLayerRow() == this;
}

bool UIEffectLayerRow::GetIsLayerVisible() const
{
    return p_visibleButton->GetOn();
}
