#include "UIEffectLayerMaskRow.h"

#include "Bang/GameObjectFactory.h"
#include "Bang/UIButton.h"
#include "Bang/UIComboBox.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILabel.h"
#include "Bang/UILayoutElement.h"
#include "BangEditor/EditorTextureFactory.h"
#include "BangEditor/UIContextMenu.h"

#include "Clipboard.h"
#include "EffectLayer.h"
#include "EffectLayerMask.h"
#include "MainScene.h"
#include "UIEffectLayerRow.h"

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

        UIHorizontalLayout *hl = innerHLGo->AddComponent<UIHorizontalLayout>();
        hl->SetPaddings(2);
        hl->SetPaddingLeft(10);
        hl->SetPaddingRight(10);
        hl->SetSpacing(5);

        p_nameLabel = GameObjectFactory::CreateUILabel();
        p_nameLabel->GetText()->SetContent("Mask");
        p_nameLabel->GetText()->SetHorizontalAlign(HorizontalAlignment::LEFT);
        p_nameLabel->GetGameObject()->SetParent(innerHLGo);

        p_maskTypeInput = GameObjectFactory::CreateUIComboBox();
        p_maskTypeInput->EventEmitter<IEventsValueChanged>::RegisterListener(
            this);
        p_maskTypeInput->AddItem("Fractal",
                                 SCAST<int>(EffectLayerMask::Type::FRACTAL));
        p_maskTypeInput->AddItem(
            "Ambient occlusion",
            SCAST<int>(EffectLayerMask::Type::AMBIENT_OCCLUSION));
        p_maskTypeInput->GetGameObject()->SetParent(innerHLGo);

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

void UIEffectLayerMaskRow::UpdateFromEffectLayerMask()
{
    p_nameLabel->GetText()->SetContent(GetEffectLayerMask()->GetName());
    p_maskTypeInput->SetSelectionByValue(
        SCAST<int>(GetEffectLayerMask()->GetType()));
}

void UIEffectLayerMaskRow::SetUIEffectLayerRow(
    UIEffectLayerRow *uiEffectLayerRow)
{
    p_uiEffectLayerRow = uiEffectLayerRow;
}

void UIEffectLayerMaskRow::SetEffectLayerMask(EffectLayerMask *effectLayerMask)
{
    p_effectLayerMask = effectLayerMask;
    p_effectLayerMask->SetName(p_nameLabel->GetText()->GetContent());
}

EffectLayerMask *UIEffectLayerMaskRow::GetEffectLayerMask() const
{
    return p_effectLayerMask;
}

UIEffectLayerRow *UIEffectLayerMaskRow::GetEffectLayerRow() const
{
    return p_uiEffectLayerRow;
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
