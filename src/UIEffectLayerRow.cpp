#include "UIEffectLayerRow.h"

#include "Bang/AARect.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Input.h"
#include "Bang/Paths.h"
#include "Bang/Random.h"
#include "Bang/RectTransform.h"
#include "Bang/UIButton.h"
#include "Bang/UIComboBox.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILabel.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UITextRenderer.h"
#include "Bang/UITheme.h"
#include "Bang/UIToolButton.h"
#include "Bang/UIVerticalLayout.h"

#include "BangEditor/EditorTextureFactory.h"
#include "BangEditor/UIContextMenu.h"

#include "Clipboard.h"
#include "ControlPanel.h"
#include "EffectLayer.h"
#include "MainScene.h"
#include "UIEffectLayers.h"
#include "View3DScene.h"

using namespace Bang;
using namespace BangEditor;

UIEffectLayerRow::UIEffectLayerRow(UIEffectLayers *uiEffectLayers,
                                   EffectLayer *effectLayer)
{
    GameObjectFactory::CreateUIGameObjectInto(this);

    SetName("UIEffectLayerRow");

    p_uiEffectLayers = uiEffectLayers;
    p_effectLayer = effectLayer;

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

    GameObjectFactory::AddInnerBorder(this);

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
    p_layerNameLabel->GetGameObject()->SetName("UIEffectLayerRowNameUILabel");
    p_layerNameLabel->GetText()->SetContent(layerName);
    p_layerNameLabel->GetText()->SetHorizontalAlign(HorizontalAlignment::LEFT);
    p_layerNameLabel->GetGameObject()->SetParent(this);

    GameObjectFactory::CreateUIHSpacer(LayoutSizeType::FLEXIBLE, 1.0f)
        ->SetParent(this);

    p_effectLayerTypeInput = GameObjectFactory::CreateUIComboBox();
    p_effectLayerTypeInput->GetGameObject()->SetName(
        "UIEffectLayerRowTypeComboBox");
    p_effectLayerTypeInput->EventEmitter<IEventsValueChanged>::RegisterListener(
        this);
    p_effectLayerTypeInput->AddItem("Dirt", EffectLayer::Type::DIRT);
    p_effectLayerTypeInput->AddItem("Normal Lines",
                                    EffectLayer::Type::NORMAL_LINES);
    p_effectLayerTypeInput->AddItem("Fractal Bumps",
                                    EffectLayer::Type::FRACTAL_BUMPS);
    p_effectLayerTypeInput->AddItem("Wave Bumps",
                                    EffectLayer::Type::WAVE_BUMPS);
    p_effectLayerTypeInput->AddItem("Ambient Occlusion",
                                    EffectLayer::Type::AMBIENT_OCCLUSION);
    p_effectLayerTypeInput->SetSelectionByValue(
        EffectLayer::Type::AMBIENT_OCCLUSION);
    p_effectLayerTypeInput->GetGameObject()->SetParent(this);

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

    p_contextMenu = AddComponent<UIContextMenu>();
    p_contextMenu->SetFocusable(p_focusable);
    p_contextMenu->SetSceneToBeAddedTo(MainScene::GetInstance());
    p_contextMenu->SetCreateContextMenuCallback([this](MenuItem *menuRootItem) {
        menuRootItem->SetFontSize(12);

        Clipboard *cb = Clipboard::GetInstance();
        MenuItem *copyMaskMenuItem = menuRootItem->AddItem("Copy Mask");
        copyMaskMenuItem->SetSelectedCallback([this, cb](MenuItem *) {
            cb->CopyMaskTexture(GetEffectLayer()->GetMaskTexture());
        });

        MenuItem *pasteMaskMenuItem = menuRootItem->AddItem("Paste Mask");
        pasteMaskMenuItem->SetSelectedCallback([this, cb](MenuItem *) {
            cb->PasteMaskTexture(GetEffectLayer()->GetMaskTexture());
        });
        pasteMaskMenuItem->SetOverAndActionEnabled(cb->HasCopiedMaskTexture());

        menuRootItem->AddSeparator();

        MenuItem *copyEffectMenuItem = menuRootItem->AddItem("Copy Effect");
        copyEffectMenuItem->SetSelectedCallback(
            [this, cb](MenuItem *) { cb->CopyEffectLayer(GetEffectLayer()); });

        MenuItem *pasteEffectMenuItem = menuRootItem->AddItem("Paste Effect");
        pasteEffectMenuItem->SetSelectedCallback([this, cb](MenuItem *) {
            cb->PasteEffectLayer(GetEffectLayer());
            UpdateFromEffectLayer();
        });

        MenuItem *duplicateEffectMenuItem =
            menuRootItem->AddItem("Duplicate Effect");
        duplicateEffectMenuItem->SetSelectedCallback(
            [this](MenuItem *) { p_uiEffectLayers->Duplicate(this); });

        pasteMaskMenuItem->SetOverAndActionEnabled(cb->HasCopiedMaskTexture());
    });
}

UIEffectLayerRow::~UIEffectLayerRow()
{
}

void UIEffectLayerRow::Update()
{
    GameObject::Update();

    if (GetEffectLayer()->GetVisible() != p_visibleButton->GetOn())
    {
        GetEffectLayer()->SetVisible(p_visibleButton->GetOn());
        MainScene::GetInstance()->GetView3DScene()->InvalidateTextures();
    }
}

void UIEffectLayerRow::UpdateFromEffectLayer()
{
    p_visibleButton->SetOn(GetEffectLayer()->GetVisible());
    p_effectLayerTypeInput->SetSelectionByValue(GetEffectLayer()->GetType());
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

UIFocusable *UIEffectLayerRow::GetFocusable() const
{
    return p_focusable;
}

EffectLayer *UIEffectLayerRow::GetEffectLayer() const
{
    return p_effectLayer;
}

UIToolButton *UIEffectLayerRow::GetIsLayerVisibleButton() const
{
    return p_visibleButton;
}

void UIEffectLayerRow::OnValueChanged(EventEmitter<IEventsValueChanged> *ee)
{
    if (ee == p_effectLayerTypeInput)
    {
        EffectLayer::Type type = SCAST<EffectLayer::Type>(
            p_effectLayerTypeInput->GetSelectedValue());
        GetEffectLayer()->SetType(type);
    }
    MainScene::GetInstance()->GetView3DScene()->InvalidateTextures();
}
