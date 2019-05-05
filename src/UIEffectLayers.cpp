#include "UIEffectLayers.h"

#include "Bang/GameObjectFactory.h"
#include "Bang/Input.h"
#include "Bang/RectTransform.h"
#include "Bang/UIButton.h"
#include "Bang/UICanvas.h"
#include "Bang/UIContentSizeFitter.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILabel.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UIButton.h"
#include "Bang/UIList.h"
#include "Bang/UIScrollArea.h"
#include "Bang/UIScrollBar.h"
#include "Bang/UIScrollPanel.h"
#include "Bang/UITextRenderer.h"
#include "Bang/UIToolButton.h"
#include "Bang/UIVerticalLayout.h"
#include "BangEditor/EditorTextureFactory.h"

#include "Clipboard.h"
#include "ControlPanel.h"
#include "MainScene.h"
#include "UIEffectLayerRow.h"
#include "View3DScene.h"

using namespace Bang;

UIEffectLayers::UIEffectLayers()
{
    GameObjectFactory::CreateUIGameObjectInto(this);

    SetName("UIEffectLayers");
    UIVerticalLayout *mainVL = AddComponent<UIVerticalLayout>();

    GameObject *topButtonsHLGo = GameObjectFactory::CreateUIGameObject();
    {
        UIImageRenderer *topRowBg =
            topButtonsHLGo->AddComponent<UIImageRenderer>();
        topRowBg->SetTint(Color::White().WithValue(0.8f));

        GameObjectFactory::AddInnerBorder(topButtonsHLGo);

        UIHorizontalLayout *topButtonsHL =
            topButtonsHLGo->AddComponent<UIHorizontalLayout>();
        topButtonsHL->SetSpacing(5);
        topButtonsHL->SetPaddings(5);

        UILabel *title = GameObjectFactory::CreateUILabel();
        title->GetText()->SetContent("Effect layers");
        title->GetText()->SetTextSize(14);
        title->GetText()->SetHorizontalAlign(HorizontalAlignment::LEFT);
        title->GetGameObject()->SetParent(topButtonsHLGo);

        GameObjectFactory::CreateUIHSpacer(LayoutSizeType::FLEXIBLE, 1.0f)
            ->SetParent(topButtonsHLGo);

        p_regenerateAllLayers =
                GameObjectFactory::CreateUIButton("Regenerate all");
        p_regenerateAllLayers->AddClickedCallback([]()
        {
            MainScene::GetInstance()->GetView3DScene()->
                    RegenerateAllEffectLayers();
        });
        p_regenerateAllLayers->GetGameObject()->SetParent(topButtonsHLGo);

        p_allLayersVisibleButton = GameObjectFactory::CreateUIToolButton(
            "", EditorTextureFactory::GetEyeIcon());
        p_allLayersVisibleButton->GetIcon()->SetTint(Color::Black());
        p_allLayersVisibleButton->AddClickedCallback([this]() {
            for (UIEffectLayerRow *layerRow : p_effectLayerRows)
            {
                layerRow->GetIsLayerVisibleButton()->SetOn(
                    p_allLayersVisibleButton->GetOn());
            }
        });
        p_allLayersVisibleButton->GetGameObject()->SetParent(topButtonsHLGo);

        UIButton *addEffectLayerButton = GameObjectFactory::CreateUIButton(
            "", EditorTextureFactory::GetAddIcon());
        addEffectLayerButton->GetIcon()->SetTint(Color::Green());
        addEffectLayerButton->AddClickedCallback([this]() {
            ControlPanel *cp = MainScene::GetInstance()->GetControlPanel();
            UIEffectLayerRow *elRow = cp->CreateNewEffectLayer();
        });
        addEffectLayerButton->GetGameObject()->SetParent(topButtonsHLGo);

        topButtonsHLGo->SetParent(this);
    }

    p_uiList = GameObjectFactory::CreateUIList(true);
    p_uiList->GetGameObject()->SetName("UIEffectLayersUIList");
    p_uiList->EventEmitter<IEventsUIList>::RegisterListener(this);

    UILayoutElement *listLE =
        p_uiList->GetGameObject()->AddComponent<UILayoutElement>();
    p_uiList->SetDragDropEnabled(true);
    listLE->SetFlexibleSize(Vector2::One());

    UIScrollPanel *scrollPanel = p_uiList->GetScrollPanel();
    scrollPanel->SetForceHorizontalFit(true);
    scrollPanel->SetHorizontalScrollEnabled(false);
    scrollPanel->SetVerticalScrollBarSide(HorizontalSide::RIGHT);

    p_uiList->GetGameObject()->SetParent(this);
}

UIEffectLayers::~UIEffectLayers()
{
}

void UIEffectLayers::Update()
{
    GameObject::Update();

    bool allLayersVisibleButtonOn = true;
    for (UIEffectLayerRow *layerRow : p_effectLayerRows)
    {
        allLayersVisibleButtonOn &= layerRow->GetIsLayerVisible();
    }
    p_allLayersVisibleButton->SetOn(allLayersVisibleButtonOn);

    if (GetRectTransform()->IsMouseOver(false))
    {
        if (GetSelectedEffectLayerRow())
        {
            if (Input::GetKey(Key::LCTRL) && Input::GetKeyDown(Key::D))
            {
                int prevSelectedIndex = p_uiList->GetSelectedIndex();
                Duplicate(GetSelectedEffectLayerRow());
                p_uiList->MoveItem(GetSelectedEffectLayerRow(),
                                   prevSelectedIndex + 1);
            }
            else if (Input::GetKeyDown(Key::DELETE))
            {
                RemoveEffectLayerRow(GetSelectedEffectLayerRowIndex(), true);
            }
        }
    }
}

UIEffectLayerRow *UIEffectLayers::CreateNewEffectLayerRow(
    EffectLayer *newEffectLayer)
{
    UIEffectLayerRow *effectLayerRow =
        new UIEffectLayerRow(this, newEffectLayer);

    p_uiList->AddItem(effectLayerRow, 0);
    p_effectLayerRows.PushFront(effectLayerRow);

    SetSelection(effectLayerRow);

    return effectLayerRow;
}

void UIEffectLayers::RemoveEffectLayerRow(uint effectLayerIdx,
                                          bool removeEffectLayer)
{
    UIEffectLayerRow *effectLayerRow = p_effectLayerRows[effectLayerIdx];
    p_uiList->RemoveItem(effectLayerRow);
    p_effectLayerRows.RemoveByIndex(effectLayerIdx);
    if (removeEffectLayer)
    {
        MainScene::GetInstance()->GetView3DScene()->RemoveEffectLayer(
            effectLayerIdx);
    }

    if (effectLayerIdx >= p_effectLayerRows.Size())
    {
        SetSelection(p_effectLayerRows.Size() - 1);
    }
    else
    {
        SetSelection(effectLayerIdx);
    }
}

void UIEffectLayers::UpdateFromEffectLayers()
{
    Clear();

    View3DScene *view3DScene = View3DScene::GetInstance();
    const auto &effectLayers = view3DScene->GetAllEffectLayers();
    for (auto it = effectLayers.RBegin(); it != effectLayers.REnd(); ++it)
    {
        EffectLayer *effectLayer = *it;
        UIEffectLayerRow *effectLayerRow = CreateNewEffectLayerRow(effectLayer);
        effectLayerRow->UpdateFromEffectLayer();
    }
}

void UIEffectLayers::SetSelection(uint idx)
{
    p_uiList->SetSelection(idx);
}

void UIEffectLayers::SetSelection(UIEffectLayerRow *effectLayerRow)
{
    SetSelection(GetUIEffectLayerRows().IndexOf(effectLayerRow));
}

void UIEffectLayers::ClearLayersSelection()
{
    SetSelection(SCAST<uint>(-1));
}

bool UIEffectLayers::IsAllLayersVisibleButtonOn() const
{
    return p_allLayersVisibleButton->GetOn();
}

void UIEffectLayers::Duplicate(UIEffectLayerRow *effectLayerRow)
{
    Clipboard *cb = Clipboard::GetInstance();
    cb->CopyEffectLayer(effectLayerRow->GetEffectLayer());
    UIEffectLayerRow *newEffectLayerRow =
        MainScene::GetInstance()->GetControlPanel()->CreateNewEffectLayer();
    cb->PasteEffectLayer(newEffectLayerRow->GetEffectLayer());
    newEffectLayerRow->UpdateFromEffectLayer();
}

void UIEffectLayers::Clear()
{
    p_uiList->Clear();
    p_effectLayerRows.Clear();
}

UIList *UIEffectLayers::GetList() const
{
    return p_uiList;
}

uint UIEffectLayers::GetSelectedEffectLayerRowIndex() const
{
    return p_uiList->GetSelectedIndex();
}

UIEffectLayerRow *UIEffectLayers::GetSelectedEffectLayerRow() const
{
    if (GetSelectedEffectLayerRowIndex() < GetUIEffectLayerRows().Size())
    {
        return GetUIEffectLayerRows()[GetSelectedEffectLayerRowIndex()];
    }
    return nullptr;
}

const Array<UIEffectLayerRow *> &UIEffectLayers::GetUIEffectLayerRows() const
{
    return p_effectLayerRows;
}

void UIEffectLayers::OnItemMoved(GameObject *item, int oldIndex, int newIndex)
{
    UIEffectLayerRow *layerRow = DCAST<UIEffectLayerRow *>(item);
    p_effectLayerRows.RemoveByIndex(oldIndex);
    p_effectLayerRows.Insert(layerRow, newIndex);

    MainScene::GetInstance()->GetView3DScene()->MoveEffectLayer(
        layerRow->GetEffectLayer(), newIndex);
}
