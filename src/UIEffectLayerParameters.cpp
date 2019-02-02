#include "UIEffectLayerParameters.h"

#include "Bang/GameObjectFactory.h"
#include "Bang/UIButton.h"
#include "Bang/UIComboBox.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILabel.h"
#include "Bang/UISlider.h"
#include "Bang/UITextRenderer.h"
#include "Bang/UIToolButton.h"
#include "Bang/UIVerticalLayout.h"
#include "BangEditor/EditorTextureFactory.h"
#include "BangEditor/UIInputColor.h"

#include "EffectLayer.h"

using namespace Bang;

UIEffectLayerParameters::UIEffectLayerParameters()
{
    GameObjectFactory::CreateUIGameObjectInto(this);

    UIVerticalLayout *vl = AddComponent<UIVerticalLayout>();
    vl->SetSpacing(5);

    auto CreateRow = [this](const String &name,
                            GameObject *valueInput,
                            UIToolButton **outVisibleToolButton,
                            UIComboBox **outBlendModeInput) {
        GameObject *row = GameObjectFactory::CreateUIGameObject();
        UILayoutElement *rowLE = row->AddComponent<UILayoutElement>();
        rowLE->SetMinHeight(25);

        UIHorizontalLayout *hl = row->AddComponent<UIHorizontalLayout>();
        hl->SetSpacing(15);

        UILabel *label = GameObjectFactory::CreateUILabel();
        label->GetText()->SetContent(name);
        label->GetText()->SetHorizontalAlign(HorizontalAlignment::LEFT);
        label->GetGameObject()->SetParent(row);
        UILayoutElement *labelLE =
            label->GetGameObject()->AddComponent<UILayoutElement>();
        labelLE->SetPreferredWidth(80);
        labelLE->SetFlexibleWidth(0.0f);

        UILayoutElement *valueInputLE =
            valueInput->AddComponent<UILayoutElement>();
        valueInputLE->SetFlexibleWidth(999.0f);
        valueInput->SetParent(row);

        UIComboBox *blendModeInput = GameObjectFactory::CreateUIComboBox();
        blendModeInput->AddItem("Add", SCAST<int>(EffectLayer::BlendMode::ADD));
        blendModeInput->AddItem("Subtract",
                                SCAST<int>(EffectLayer::BlendMode::SUBTRACT));
        blendModeInput->AddItem("Multiply",
                                SCAST<int>(EffectLayer::BlendMode::MULTIPLY));
        blendModeInput->EventEmitter<IEventsValueChanged>::RegisterListener(
            this);

        UILayoutElement *blendModeInputLE =
            blendModeInput->GetGameObject()->AddComponent<UILayoutElement>();
        blendModeInputLE->SetMinWidth(80);
        blendModeInputLE->SetLayoutPriority(99);
        blendModeInput->GetGameObject()->SetParent(row);

        UIToolButton *visibleToolButton = GameObjectFactory::CreateUIToolButton(
            "", EditorTextureFactory::GetEyeIcon());
        visibleToolButton->GetIcon()->SetTint(Color::Black());
        visibleToolButton->AddClickedCallback(
            [this]() { OnValueChanged(nullptr); });
        visibleToolButton->SetOn(true);
        visibleToolButton->GetGameObject()->SetParent(row);

        *outBlendModeInput = blendModeInput;
        *outVisibleToolButton = visibleToolButton;
        return row;
    };

    p_colorInput = new UIInputColor();
    p_colorInput->GetSearchColorButton()->GetGameObject()->SetEnabled(false);
    p_colorInput->EventEmitter<IEventsValueChanged>::RegisterListener(this);
    CreateRow(
        "Color", p_colorInput, &p_colorVisibleButton, &p_colorBlendModeInput)
        ->SetParent(this);

    p_heightInput = GameObjectFactory::CreateUISlider(-1.0f, 1.0f, 0.1f);
    p_heightInput->EventEmitter<IEventsValueChanged>::RegisterListener(this);
    CreateRow("Height",
              p_heightInput->GetGameObject(),
              &p_heightVisibleButton,
              &p_heightBlendModeInput)
        ->SetParent(this);

    p_roughnessInput = GameObjectFactory::CreateUISlider(0.0f, 1.0f, 0.1f);
    p_roughnessInput->EventEmitter<IEventsValueChanged>::RegisterListener(this);
    CreateRow("Roughness",
              p_roughnessInput->GetGameObject(),
              &p_roughnessVisibleButton,
              &p_roughnessBlendModeInput)
        ->SetParent(this);

    p_metalnessInput = GameObjectFactory::CreateUISlider(0.0f, 1.0f, 0.1f);
    p_metalnessInput->EventEmitter<IEventsValueChanged>::RegisterListener(this);
    CreateRow("Metalness",
              p_metalnessInput->GetGameObject(),
              &p_metalnessVisibleButton,
              &p_metalnessBlendModeInput)
        ->SetParent(this);
}

UIEffectLayerParameters::~UIEffectLayerParameters()
{
}

void UIEffectLayerParameters::SetEffectLayer(EffectLayer *effectLayer)
{
    if (effectLayer != GetEffectLayer())
    {
        p_effectLayer = effectLayer;

        EventListener<IEventsValueChanged>::SetReceiveEvents(false);

        p_colorBlendModeInput->SetSelectionByValue(
            SCAST<int>(effectLayer->GetColorBlendMode()));
        p_colorVisibleButton->SetOn(effectLayer->GetColorBlendMode() !=
                                    EffectLayer::BlendMode::IGNORE);
        p_heightBlendModeInput->SetSelectionByValue(
            SCAST<int>(effectLayer->GetHeightBlendMode()));
        p_heightVisibleButton->SetOn(effectLayer->GetHeightBlendMode() !=
                                     EffectLayer::BlendMode::IGNORE);
        p_roughnessBlendModeInput->SetSelectionByValue(
            SCAST<int>(effectLayer->GetRoughnessBlendMode()));
        p_roughnessVisibleButton->SetOn(effectLayer->GetRoughnessBlendMode() !=
                                        EffectLayer::BlendMode::IGNORE);
        p_metalnessBlendModeInput->SetSelectionByValue(
            SCAST<int>(effectLayer->GetMetalnessBlendMode()));
        p_metalnessVisibleButton->SetOn(effectLayer->GetMetalnessBlendMode() !=
                                        EffectLayer::BlendMode::IGNORE);

        p_colorInput->SetColor(effectLayer->GetColor());
        p_heightInput->SetValue(effectLayer->GetHeight());
        p_roughnessInput->SetValue(effectLayer->GetRoughness());
        p_metalnessInput->SetValue(effectLayer->GetMetalness());

        EventListener<IEventsValueChanged>::SetReceiveEvents(true);
    }
}

EffectLayer *UIEffectLayerParameters::GetEffectLayer() const
{
    return p_effectLayer;
}

void UIEffectLayerParameters::OnValueChanged(
    EventEmitter<IEventsValueChanged> *)
{
    if (p_colorVisibleButton->GetOn())
    {
        p_effectLayer->SetColorBlendMode(SCAST<EffectLayer::BlendMode>(
            p_colorBlendModeInput->GetSelectedValue()));
    }
    else
    {
        p_effectLayer->SetColorBlendMode(EffectLayer::BlendMode::IGNORE);
    }

    if (p_heightVisibleButton->GetOn())
    {
        p_effectLayer->SetHeightBlendMode(SCAST<EffectLayer::BlendMode>(
            p_heightBlendModeInput->GetSelectedValue()));
    }
    else
    {
        p_effectLayer->SetHeightBlendMode(EffectLayer::BlendMode::IGNORE);
    }

    if (p_roughnessVisibleButton->GetOn())
    {
        p_effectLayer->SetRoughnessBlendMode(SCAST<EffectLayer::BlendMode>(
            p_roughnessBlendModeInput->GetSelectedValue()));
    }
    else
    {
        p_effectLayer->SetRoughnessBlendMode(EffectLayer::BlendMode::IGNORE);
    }

    if (p_metalnessVisibleButton->GetOn())
    {
        p_effectLayer->SetMetalnessBlendMode(SCAST<EffectLayer::BlendMode>(
            p_metalnessBlendModeInput->GetSelectedValue()));
    }
    else
    {
        p_effectLayer->SetMetalnessBlendMode(EffectLayer::BlendMode::IGNORE);
    }

    p_effectLayer->SetColor(p_colorInput->GetColor());
    p_effectLayer->SetHeight(p_heightInput->GetValue());
    p_effectLayer->SetRoughness(p_roughnessInput->GetValue());
    p_effectLayer->SetMetalness(p_metalnessInput->GetValue());
}
