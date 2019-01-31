#include "UIEffectLayerMaskRow.h"

#include "Bang/GameObjectFactory.h"
#include "Bang/UIComboBox.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILabel.h"
#include "Bang/UILayoutElement.h"

#include "EffectLayer.h"

using namespace Bang;

UIEffectLayerMaskRow::UIEffectLayerMaskRow()
{
    GameObjectFactory::CreateUIGameObjectInto(this);

    UIHorizontalLayout *mainHL = AddComponent<UIHorizontalLayout>();

    GameObject *leftMarginGo = GameObjectFactory::CreateUIGameObject();
    {
        UILayoutElement *marginLE =
            leftMarginGo->AddComponent<UILayoutElement>();
        marginLE->SetMinWidth(30);

        UIImageRenderer *leftMarginImg =
            leftMarginGo->AddComponent<UIImageRenderer>();
        leftMarginImg->SetTint(Color::White());
    }
    leftMarginGo->SetParent(this);

    GameObject *innerHLGo = GameObjectFactory::CreateUIGameObject();
    {
        UILayoutElement *le = innerHLGo->AddComponent<UILayoutElement>();
        le->SetFlexibleWidth(1.0f);

        UIHorizontalLayout *hl = innerHLGo->AddComponent<UIHorizontalLayout>();
        hl->SetPaddings(2);
        hl->SetPaddingLeft(10);
        hl->SetPaddingRight(10);

        UILabel *label = GameObjectFactory::CreateUILabel();
        label->GetText()->SetContent("Mask");
        label->GetText()->SetHorizontalAlign(HorizontalAlignment::LEFT);
        label->GetGameObject()->SetParent(innerHLGo);

        p_maskTypeInput = GameObjectFactory::CreateUIComboBox();
        p_maskTypeInput->EventEmitter<IEventsValueChanged>::RegisterListener(
            this);
        p_maskTypeInput->AddItem("Fractal",
                                 SCAST<int>(EffectLayer::MaskType::FRACTAL));
        p_maskTypeInput->AddItem(
            "Normal based", SCAST<int>(EffectLayer::MaskType::NORMAL_BASED));
        p_maskTypeInput->AddItem(
            "Ambient occlusion",
            SCAST<int>(EffectLayer::MaskType::AMBIENT_OCCLUSION));
        p_maskTypeInput->GetGameObject()->SetParent(innerHLGo);
    }

    GameObjectFactory::AddInnerBorder(
        innerHLGo, Vector2i(1), Color::White().WithValue(0.3f));
    innerHLGo->SetParent(this);
}

UIEffectLayerMaskRow::~UIEffectLayerMaskRow()
{
}

void UIEffectLayerMaskRow::OnValueChanged(EventEmitter<IEventsValueChanged> *ee)
{
    if (ee == p_maskTypeInput)
    {
        // switch (p_maskTypeInput)
        {
        }
    }
}
