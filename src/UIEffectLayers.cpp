#include "UIEffectLayers.h"

#include "Bang/GameObjectFactory.h"
#include "Bang/RectTransform.h"
#include "Bang/UIButton.h"
#include "Bang/UIContentSizeFitter.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILabel.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UIList.h"
#include "Bang/UIScrollArea.h"
#include "Bang/UIScrollBar.h"
#include "Bang/UIScrollPanel.h"
#include "Bang/UITextRenderer.h"
#include "Bang/UIToolButton.h"
#include "Bang/UIVerticalLayout.h"
#include "BangEditor/EditorTextureFactory.h"

#include "ControlPanel.h"
#include "MainScene.h"
#include "UIEffectLayerRow.h"
#include "View3DScene.h"

using namespace Bang;

UIEffectLayers::UIEffectLayers()
{
    GameObjectFactory::CreateUIGameObjectInto(this);

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

        UIButton *addEffectLayerButton = GameObjectFactory::CreateUIButton(
            "", EditorTextureFactory::GetAddIcon());
        addEffectLayerButton->GetIcon()->SetTint(Color::Green());
        addEffectLayerButton->AddClickedCallback([this]() {
            ControlPanel *cp = MainScene::GetInstance()->GetControlPanel();
            cp->CreateNewEffectLayer();
        });
        addEffectLayerButton->GetGameObject()->SetParent(topButtonsHLGo);

        p_allLayersVisibleButton = GameObjectFactory::CreateUIToolButton(
            "", EditorTextureFactory::GetEyeIcon());
        p_allLayersVisibleButton->GetIcon()->SetTint(Color::Black());
        p_allLayersVisibleButton->AddClickedCallback([this]() {
            for (UIEffectLayerRow *layerRow : p_effectLayerRows)
            {
                layerRow->GetIsLayerVisibleButton()->SetOn(
                    p_allLayersVisibleButton->GetOn());
            }
            MainScene::GetInstance()->GetView3DScene()->InvalidateTextures();
        });
        p_allLayersVisibleButton->GetGameObject()->SetParent(topButtonsHLGo);

        topButtonsHLGo->SetParent(this);
    }

    p_scrollPanel = GameObjectFactory::CreateUIScrollPanel();
    {
        UILayoutElement *scrollLE =
            p_scrollPanel->GetGameObject()->AddComponent<UILayoutElement>();
        scrollLE->SetFlexibleSize(Vector2(1.0f));
        p_scrollPanel->SetForceHorizontalFit(true);
        p_scrollPanel->SetHorizontalScrollEnabled(false);
        p_scrollPanel->SetVerticalScrollBarSide(HorizontalSide::RIGHT);
        p_scrollPanel->GetGameObject()->SetParent(this);
    }

    p_listContainer = GameObjectFactory::CreateUIGameObject();
    {
        UIVerticalLayout *vl =
            p_listContainer->AddComponent<UIVerticalLayout>();
        p_scrollPanel->GetScrollArea()->SetContainedGameObject(p_listContainer);
    }
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
}

UIEffectLayerRow *UIEffectLayers::CreateNewEffectLayerRow(
    EffectLayer *newEffectLayer)
{
    UIEffectLayerRow *effectLayerRow =
        new UIEffectLayerRow(this, newEffectLayer);

    effectLayerRow->SetParent(p_listContainer, 0);
    p_effectLayerRows.PushFront(effectLayerRow);

    SetSelection(effectLayerRow);

    return effectLayerRow;
}

void UIEffectLayers::RemoveEffectLayer(uint effectLayerIdx)
{
    UIEffectLayerRow *eflRow = p_effectLayerRows[effectLayerIdx];
    GameObject::Destroy(eflRow);
    p_effectLayerRows.RemoveByIndex(effectLayerIdx);
}

void UIEffectLayers::SetSelection(uint idx)
{
    m_selectedEffectLayerRowIndex = idx;

    ControlPanel *cpanel = MainScene::GetInstance()->GetControlPanel();
    cpanel->UpdateInputsAndParametersFromSelectedEffectLayer();
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

uint UIEffectLayers::GetSelectedEffectLayerRowIndex() const
{
    return m_selectedEffectLayerRowIndex;
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
