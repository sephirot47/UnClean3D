#include "UIEffectLayerMaskRow.h"

#include "Bang/GameObjectFactory.h"
#include "Bang/Input.h"
#include "Bang/UIButton.h"
#include "Bang/UIComboBox.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILabel.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UIToolButton.h"
#include "BangEditor/EditorTextureFactory.h"
#include "BangEditor/UIContextMenu.h"

#include "Clipboard.h"
#include "EffectLayer.h"
#include "EffectLayerMask.h"
#include "MainScene.h"
#include "UIEffectLayerRow.h"
#include "UIEffectLayers.h"

using namespace Bang;
using namespace BangEditor;

UIEffectLayerMaskRow::UIEffectLayerMaskRow(UIEffectLayerRow *uiEffectLayerRow,
                                           EffectLayerMask *effectLayerMask)
{
    p_uiEffectLayerRow = uiEffectLayerRow;
    p_effectLayerMask = effectLayerMask;

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

    UIFocusable *focusable = AddComponent<UIFocusable>();

    p_contextMenu = AddComponent<UIContextMenu>();
    p_contextMenu->SetFocusable(focusable);
    p_contextMenu->SetSceneToBeAddedTo(MainScene::GetInstance());
    p_contextMenu->SetCreateContextMenuCallback([this](MenuItem *menuRootItem) {
        menuRootItem->SetFontSize(12);

        Clipboard *cb = Clipboard::GetInstance();
        MenuItem *copyMaskMenuItem = menuRootItem->AddItem("Copy Mask");
        copyMaskMenuItem->SetSelectedCallback([this, cb](MenuItem *) {
            cb->CopyEffectLayerMask(GetEffectLayerMask());
        });

        MenuItem *pasteMaskMenuItem = menuRootItem->AddItem("Paste Mask");
        pasteMaskMenuItem->SetSelectedCallback([this, cb](MenuItem *) {
            cb->PasteEffectLayerMask(GetEffectLayerMask());
        });
        pasteMaskMenuItem->SetOverAndActionEnabled(
            cb->HasCopiedEffectLayerMask());

        MenuItem *duplicateMaskMenuItem =
            menuRootItem->AddItem("Duplicate Mask");
        duplicateMaskMenuItem->SetSelectedCallback(
            [this](MenuItem *) { GetEffectLayerRow()->Duplicate(this); });

        pasteMaskMenuItem->SetOverAndActionEnabled(
            cb->HasCopiedEffectLayerMask());
    });

    GameObject *innerHLGo = GameObjectFactory::CreateUIGameObject();
    {
        UILayoutElement *le = innerHLGo->AddComponent<UILayoutElement>();
        le->SetFlexibleWidth(1.0f);
        le->SetMinHeight(20);

        UIHorizontalLayout *hl = innerHLGo->AddComponent<UIHorizontalLayout>();
        hl->SetPaddings(0);
        hl->SetPaddingLeft(10);
        hl->SetPaddingRight(10);
        hl->SetSpacing(5);

        p_nameLabel = GameObjectFactory::CreateUILabel();
        p_nameLabel->GetText()->SetContent("Mask");
        p_nameLabel->GetText()->SetHorizontalAlign(HorizontalAlignment::LEFT);
        p_nameLabel->SetFloatingInputEnabled(true);
        p_nameLabel->EventEmitter<IEventsUILabel>::RegisterListener(this);
        p_nameLabel->GetGameObject()->SetParent(innerHLGo);

        p_maskTypeInput = GameObjectFactory::CreateUIComboBox();
        p_maskTypeInput->AddItem("Simplex Noise",
                                 SCAST<int>(EffectLayerMask::Type::SIMPLEX_NOISE));
        p_maskTypeInput->AddItem("Cells Noise",
                                 SCAST<int>(EffectLayerMask::Type::CELLS_NOISE));
        p_maskTypeInput->AddItem("Cracks Noise",
                                 SCAST<int>(EffectLayerMask::Type::CRACKS_NOISE));
        p_maskTypeInput->AddItem("Polka Dots Noise",
                                 SCAST<int>(EffectLayerMask::Type::POLKA_DOTS_NOISE));
        p_maskTypeInput->AddItem("Scratches Noise",
                                 SCAST<int>(EffectLayerMask::Type::SCRATCHES_NOISE));
        p_maskTypeInput->AddItem("White noise",
                                 SCAST<int>(EffectLayerMask::Type::WHITE_NOISE));
        p_maskTypeInput->AddItem(
            "Ambient occlusion",
            SCAST<int>(EffectLayerMask::Type::AMBIENT_OCCLUSION));
        p_maskTypeInput->AddItem("Edges",
                                 SCAST<int>(EffectLayerMask::Type::EDGES));
        p_maskTypeInput->AddItem("Brush",
                                 SCAST<int>(EffectLayerMask::Type::BRUSH));
        p_maskTypeInput->AddItem("Normal",
                                 SCAST<int>(EffectLayerMask::Type::NORMAL));
        p_maskTypeInput->AddItem("Blur",
                                 SCAST<int>(EffectLayerMask::Type::BLUR));
        p_maskTypeInput->SetSelectionByValue(
            SCAST<int>(EffectLayerMask::Type::SIMPLEX_NOISE));
        p_maskTypeInput->EventEmitter<IEventsValueChanged>::RegisterListener(
            this);
        p_maskTypeInput->GetGameObject()->SetParent(innerHLGo);

        p_blendModeInput = GameObjectFactory::CreateUIComboBox();
        p_blendModeInput->AddItem("Add",
                                  SCAST<int>(EffectLayerMask::BlendMode::ADD));
        p_blendModeInput->AddItem(
            "Subtract", SCAST<int>(EffectLayerMask::BlendMode::SUBTRACT));
        p_blendModeInput->AddItem(
            "Multiply", SCAST<int>(EffectLayerMask::BlendMode::MULTIPLY));
        p_blendModeInput->SetSelectionByValue(
            SCAST<int>(EffectLayerMask::BlendMode::ADD));
        p_blendModeInput->EventEmitter<IEventsValueChanged>::RegisterListener(
            this);
        p_blendModeInput->GetGameObject()->SetParent(innerHLGo);

        p_visibleButton = GameObjectFactory::CreateUIToolButton(
            "", EditorTextureFactory::GetEyeIcon());
        p_visibleButton->GetIcon()->SetTint(Color::Black());
        p_visibleButton->SetOn(true);
        p_visibleButton->AddClickedCallback([this]() {
            GetEffectLayerMask()->SetVisible(p_visibleButton->GetOn());
        });
        p_visibleButton->GetGameObject()->SetParent(innerHLGo);

        p_removeButton = GameObjectFactory::CreateUIButton(
            "", EditorTextureFactory::GetLessIcon());
        p_removeButton->GetIcon()->SetTint(Color::Red());
        p_removeButton->AddClickedCallback(
            [this]() { GetEffectLayerRow()->RemoveMaskRow(this); });
        p_removeButton->GetGameObject()->SetParent(innerHLGo);
    }

    GameObjectFactory::AddInnerBorder(
        innerHLGo, Vector2i(1), Color::White().WithValue(0.3f));
    innerHLGo->SetParent(this);
}

UIEffectLayerMaskRow::~UIEffectLayerMaskRow()
{
}

void UIEffectLayerMaskRow::Update()
{
    GameObject::Update();

    if (Input::GetKey(Key::F2))
    {
        if (GetEffectLayerRow()->GetMaskRowsList()->GetSelectedItem() == this)
        {
            GetNameLabel()->ShowFloatingInputText();
        }
    }
    UpdateEffectLayerMaskFromUI();
}

void UIEffectLayerMaskRow::UpdateFromEffectLayerMask()
{
    this->EventListener<IEventsValueChanged>::SetReceiveEvents(false);

    GetNameLabel()->GetText()->SetContent(GetEffectLayerMask()->GetName());
    p_maskTypeInput->SetSelectionByValue(
        SCAST<int>(GetEffectLayerMask()->GetType()));
    p_blendModeInput->SetSelectionByValue(
        SCAST<int>(GetEffectLayerMask()->GetBlendMode()));

    this->EventListener<IEventsValueChanged>::SetReceiveEvents(true);
}

void UIEffectLayerMaskRow::UpdateEffectLayerMaskFromUI()
{
    GetEffectLayerMask()->SetType(
        SCAST<EffectLayerMask::Type>(p_maskTypeInput->GetSelectedValue()));
    GetEffectLayerMask()->SetBlendMode(SCAST<EffectLayerMask::BlendMode>(
        p_blendModeInput->GetSelectedValue()));
}

void UIEffectLayerMaskRow::SetUIEffectLayerRow(
    UIEffectLayerRow *uiEffectLayerRow)
{
    p_uiEffectLayerRow = uiEffectLayerRow;
}

void UIEffectLayerMaskRow::SetEffectLayerMask(EffectLayerMask *effectLayerMask)
{
    p_effectLayerMask = effectLayerMask;
}

EffectLayerMask *UIEffectLayerMaskRow::GetEffectLayerMask() const
{
    return p_effectLayerMask;
}

UIEffectLayerRow *UIEffectLayerMaskRow::GetEffectLayerRow() const
{
    return p_uiEffectLayerRow;
}

UILabel *UIEffectLayerMaskRow::GetNameLabel() const
{
    return p_nameLabel;
}

void UIEffectLayerMaskRow::OnFloatingInputTextCommited(
    const String &commitedText)
{
    GetEffectLayerMask()->SetName(commitedText);
}

void UIEffectLayerMaskRow::OnValueChanged(EventEmitter<IEventsValueChanged> *ee)
{
    UpdateEffectLayerMaskFromUI();
}
