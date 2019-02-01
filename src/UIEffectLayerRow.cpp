#include "UIEffectLayerRow.h"

#include "Bang/AARect.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Input.h"
#include "Bang/Paths.h"
#include "Bang/Random.h"
#include "Bang/RectTransform.h"
#include "Bang/UIButton.h"
#include "Bang/UICanvas.h"
#include "Bang/UIComboBox.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILabel.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UIList.h"
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
#include "UIEffectLayerMaskRow.h"
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

    UIVerticalLayout *mainVL = AddComponent<UIVerticalLayout>();

    GameObject *effectRow = GameObjectFactory::CreateUIGameObject();
    {
        UIHorizontalLayout *hl = effectRow->AddComponent<UIHorizontalLayout>();
        hl->SetSpacing(5);
        hl->SetPaddings(2);
        hl->SetPaddingRight(10);
        hl->SetPaddingLeft(10);

        p_focusable = AddComponent<UIFocusable>();
        p_focusable->AddEventCallback(
            [this](UIFocusable *, const UIEvent &event) {
                switch (event.type)
                {
                    case UIEvent::Type::MOUSE_CLICK_DOWN:
                        p_uiEffectLayers->SetSelection(this);
                        return UIEventResult::IGNORE;
                    default: break;
                }
                return UIEventResult::IGNORE;
            });

        GameObjectFactory::AddOuterBorder(effectRow);

        Array<String> existingLayerNames;
        for (UIEffectLayerRow *effectLayerRow :
             p_uiEffectLayers->GetUIEffectLayerRows())
        {
            existingLayerNames.PushBack(effectLayerRow->GetName());
        }
        String layerName =
            Path::GetDuplicateString("NewLayer", existingLayerNames);

        p_layerNameLabel = GameObjectFactory::CreateUILabel();
        p_layerNameLabel->GetGameObject()->SetName(
            "UIEffectLayerRowNameUILabel");
        p_layerNameLabel->GetText()->SetContent(layerName);
        p_layerNameLabel->GetText()->SetHorizontalAlign(
            HorizontalAlignment::LEFT);
        p_layerNameLabel->GetGameObject()->SetParent(effectRow);

        GameObjectFactory::CreateUIHSpacer(LayoutSizeType::FLEXIBLE, 1.0f)
            ->SetParent(effectRow);

        p_effectLayerTypeInput = GameObjectFactory::CreateUIComboBox();
        p_effectLayerTypeInput->GetGameObject()->SetName(
            "UIEffectLayerRowTypeComboBox");
        p_effectLayerTypeInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);
        p_effectLayerTypeInput->AddItem("Dirt",
                                        SCAST<int>(EffectLayer::Type::DIRT));
        p_effectLayerTypeInput->AddItem(
            "Normal Lines", SCAST<int>(EffectLayer::Type::NORMAL_LINES));
        p_effectLayerTypeInput->AddItem(
            "Fractal Bumps", SCAST<int>(EffectLayer::Type::FRACTAL_BUMPS));
        p_effectLayerTypeInput->AddItem(
            "Wave Bumps", SCAST<int>(EffectLayer::Type::WAVE_BUMPS));
        p_effectLayerTypeInput->AddItem(
            "Ambient Occlusion",
            SCAST<int>(EffectLayer::Type::AMBIENT_OCCLUSION));
        p_effectLayerTypeInput->AddItem(
            "Ambient Occlusion GPU",
            SCAST<int>(EffectLayer::Type::AMBIENT_OCCLUSION_GPU));
        p_effectLayerTypeInput->SetSelectionByValue(
            SCAST<int>(EffectLayer::Type::DIRT));
        p_effectLayerTypeInput->GetGameObject()->SetParent(effectRow);

        p_visibleButton = GameObjectFactory::CreateUIToolButton(
            "", EditorTextureFactory::GetEyeIcon());
        p_visibleButton->GetIcon()->SetTint(Color::Black());
        p_visibleButton->SetOn(true);
        p_visibleButton->GetGameObject()->SetParent(effectRow);

        UIButton *removeLayerButton = GameObjectFactory::CreateUIButton(
            "", EditorTextureFactory::GetLessIcon());
        removeLayerButton->GetIcon()->SetTint(Color::Red());
        removeLayerButton->AddClickedCallback([this]() {
            ControlPanel *cp = MainScene::GetInstance()->GetControlPanel();
            uint idx = p_uiEffectLayers->GetUIEffectLayerRows().IndexOf(this);
            cp->RemoveEffectLayer(idx);
        });
        removeLayerButton->GetGameObject()->SetParent(effectRow);

        p_contextMenu = AddComponent<UIContextMenu>();
        p_contextMenu->SetFocusable(p_focusable);
        p_contextMenu->SetSceneToBeAddedTo(MainScene::GetInstance());
        p_contextMenu->SetCreateContextMenuCallback([this](
            MenuItem *menuRootItem) {
            menuRootItem->SetFontSize(12);

            Clipboard *cb = Clipboard::GetInstance();
            MenuItem *copyMaskMenuItem = menuRootItem->AddItem("Copy Mask");
            copyMaskMenuItem->SetSelectedCallback([this, cb](MenuItem *) {
                cb->CopyMaskTexture(GetEffectLayer()->GetMergedMaskTexture());
            });

            MenuItem *pasteMaskMenuItem = menuRootItem->AddItem("Paste Mask");
            pasteMaskMenuItem->SetSelectedCallback([this, cb](MenuItem *) {
                cb->PasteMaskTexture(GetEffectLayer()->GetMergedMaskTexture());
            });
            pasteMaskMenuItem->SetOverAndActionEnabled(
                cb->HasCopiedMaskTexture());

            menuRootItem->AddSeparator();

            MenuItem *copyEffectMenuItem = menuRootItem->AddItem("Copy Effect");
            copyEffectMenuItem->SetSelectedCallback([this, cb](MenuItem *) {
                cb->CopyEffectLayer(GetEffectLayer());
            });

            MenuItem *pasteEffectMenuItem =
                menuRootItem->AddItem("Paste Effect");
            pasteEffectMenuItem->SetSelectedCallback([this, cb](MenuItem *) {
                cb->PasteEffectLayer(GetEffectLayer());
                UpdateFromEffectLayer();
            });

            MenuItem *duplicateEffectMenuItem =
                menuRootItem->AddItem("Duplicate Effect");
            duplicateEffectMenuItem->SetSelectedCallback(
                [this](MenuItem *) { p_uiEffectLayers->Duplicate(this); });

            pasteMaskMenuItem->SetOverAndActionEnabled(
                cb->HasCopiedMaskTexture());
        });
    }

    effectRow->SetParent(this);

    p_maskRowsList = GameObjectFactory::CreateUIList(false);

    p_maskRowsList->SetIdleColor(Color::White());
    p_maskRowsList->ClearSelection();

    p_maskRowsList->GetGameObject()->SetParent(this);

    p_addNewMaskRow = GameObjectFactory::CreateUIGameObject();
    {
        UIHorizontalLayout *hl =
            p_addNewMaskRow->AddComponent<UIHorizontalLayout>();
        hl->SetSpacing(5);
        hl->SetPaddings(5);
        hl->SetPaddingRight(15);

        UIImageRenderer *bg = p_addNewMaskRow->AddComponent<UIImageRenderer>();
        bg->SetTint(Color::White());

        UILabel *addLabel = GameObjectFactory::CreateUILabel();
        addLabel->GetText()->SetContent("Add New Mask");
        addLabel->GetText()->SetHorizontalAlign(HorizontalAlignment::RIGHT);
        addLabel->GetGameObject()->SetParent(p_addNewMaskRow);

        p_addNewMaskButton = GameObjectFactory::CreateUIButton(
            "", EditorTextureFactory::GetAddIcon());
        p_addNewMaskButton->GetIcon()->SetTint(Color::Green());
        p_addNewMaskButton->GetGameObject()->SetParent(p_addNewMaskRow);

        p_addNewMaskButton->AddClickedCallback([this]() {
            EffectLayerMask *layerMask = GetEffectLayer()->AddNewMask();

            UIEffectLayerMaskRow *maskRow = new UIEffectLayerMaskRow();
            maskRow->SetUIEffectLayerRow(this);
            maskRow->SetEffectLayerMask(layerMask);
            p_maskRowsList->AddItem(maskRow);
            p_maskRowsList->SetSelection(maskRow);
            p_maskRows.PushBack(maskRow);

        });
    }
    p_addNewMaskRow->SetParent(this);
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

    if (!IsSelected())
    {
        p_maskRowsList->ClearSelection();
    }
    else
    {
        if (p_maskRowsList->GetSelectedItem())
        {
            if (GetFocusable()->IsMouseOver())
            {
                p_uiEffectLayers->GetList()->SetSelectedColor(
                    UITheme::GetOverColor());
                if (GetFocusable()->IsBeingPressed())
                {
                    p_uiEffectLayers->GetList()->SetSelectedColor(
                        UITheme::GetSelectedColor());
                    p_maskRowsList->ClearSelection();
                }
            }
            else
            {
                p_uiEffectLayers->GetList()->SetSelectedColor(Color::White());
            }
        }
        else
        {
            p_uiEffectLayers->GetList()->SetIdleColor(Color::White());
            p_uiEffectLayers->GetList()->SetOverColor(UITheme::GetOverColor());
            p_uiEffectLayers->GetList()->SetSelectedColor(
                UITheme::GetSelectedColor());
        }
    }

    bool somethingBeingDragged =
        UICanvas::GetActive(this)->GetCurrentDragDroppable();
    bool showMaskRows = (!somethingBeingDragged && IsSelected());
    for (UIEffectLayerMaskRow *maskRow : p_maskRows)
    {
        maskRow->SetEnabled(showMaskRows);
    }
    p_addNewMaskRow->SetEnabled(showMaskRows);
}

void UIEffectLayerRow::UpdateFromEffectLayer()
{
    p_visibleButton->SetOn(GetEffectLayer()->GetVisible());
    p_effectLayerTypeInput->SetSelectionByValue(
        SCAST<int>(GetEffectLayer()->GetType()));
}

void UIEffectLayerRow::RemoveMaskRow(UIEffectLayerMaskRow *maskRow)
{
    p_maskRowsList->RemoveItem(maskRow);
    p_maskRows.Remove(maskRow);
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

UIList *UIEffectLayerRow::GetMaskRowsList() const
{
    return p_maskRowsList;
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
