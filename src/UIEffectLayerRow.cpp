#include "UIEffectLayerRow.h"

#include "Bang/AARect.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Input.h"
#include "Bang/Paths.h"
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

#include "BangEditor/EditorTextureFactory.h"

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
    le->SetMinHeight(32);

    Array<String> existingLayerNames;
    for (UIEffectLayerRow *effectLayerRow :
         p_uiEffectLayers->GetUIEffectLayerRows())
    {
        existingLayerNames.PushBack(effectLayerRow->GetName());
    }
    String layerName = Path::GetDuplicateString("NewLayer", existingLayerNames);

    p_layerNameLabel = GameObjectFactory::CreateUILabel();
    p_layerNameLabel->GetText()->SetContent(layerName);
    p_layerNameLabel->GetText()->SetHorizontalAlign(HorizontalAlignment::LEFT);
    p_layerNameLabel->GetGameObject()->SetParent(this);

    GameObjectFactory::CreateUIHSpacer(LayoutSizeType::FLEXIBLE, 1.0f)
        ->SetParent(this);

    p_visibleButton = GameObjectFactory::CreateUIToolButton(
        "", EditorTextureFactory::GetEyeIcon());
    p_visibleButton->GetIcon()->SetTint(Color::Black());
    p_visibleButton->SetOn(true);
    p_visibleButton->GetGameObject()->SetParent(this);

    UIButton *removeLayerButton = GameObjectFactory::CreateUIButton(
        "", EditorTextureFactory::GetLessIcon());
    removeLayerButton->GetIcon()->SetTint(Color::Red());
    removeLayerButton->AddClickedCallback([this]() {
        ControlPanel *cp = MainScene::GetInstance()->GetControlPanel();
        uint idx = p_uiEffectLayers->GetUIEffectLayerRows().IndexOf(this);
        cp->RemoveEffectLayer(idx);
    });
    removeLayerButton->GetGameObject()->SetParent(this);
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
        if (p_focusable->IsMouseOver())
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

String UIEffectLayerRow::GetName() const
{
    return p_layerNameLabel->GetText()->GetContent();
}

bool UIEffectLayerRow::IsSelected() const
{
    return p_uiEffectLayers->GetSelectedEffectLayerRow() == this;
}

bool UIEffectLayerRow::GetIsLayerVisible() const
{
    return p_visibleButton->GetOn();
}
