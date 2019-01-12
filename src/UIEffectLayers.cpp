#include "UIEffectLayers.h"

#include "Bang/GameObjectFactory.h"
#include "Bang/UIButton.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILabel.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UIScrollArea.h"
#include "Bang/UIScrollBar.h"
#include "Bang/UIScrollPanel.h"
#include "Bang/UITextRenderer.h"
#include "Bang/UIVerticalLayout.h"

#include "ControlPanel.h"
#include "MainScene.h"
#include "UIEffectLayerRow.h"

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

        GameObjectFactory::CreateUIHSpacer(LayoutSizeType::FLEXIBLE, 1.0f)
            ->SetParent(topButtonsHLGo);

        UIButton *addEffectLayerButton = GameObjectFactory::CreateUIButton("+");
        addEffectLayerButton->AddClickedCallback([this]() {
            ControlPanel *cp = MainScene::GetInstance()->GetControlPanel();
            cp->CreateNewEffectLayer();
        });
        addEffectLayerButton->GetGameObject()->SetParent(topButtonsHLGo);

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

        p_scrollPanelContainer = p_scrollPanel->GetScrollArea()->GetContainer();
        UIVerticalLayout *scrollContVL =
            p_scrollPanelContainer->AddComponent<UIVerticalLayout>();
        scrollContVL->SetSpacing(0);
    }
}

UIEffectLayers::~UIEffectLayers()
{
}

UIEffectLayerRow *UIEffectLayers::CreateNewEffectLayerRow()
{
    UIEffectLayerRow *effectLayerRow = new UIEffectLayerRow();

    effectLayerRow->SetParent(p_scrollPanelContainer);

    p_effectLayerRows.PushBack(effectLayerRow);
    return effectLayerRow;
}
