#include "UIEffectLayers.h"

#include "Bang/GameObjectFactory.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILabel.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UIScrollArea.h"
#include "Bang/UIScrollBar.h"
#include "Bang/UIScrollPanel.h"
#include "Bang/UITextRenderer.h"
#include "Bang/UIVerticalLayout.h"

#include "UIEffectLayerRow.h"

using namespace Bang;

UIEffectLayers::UIEffectLayers()
{
    GameObjectFactory::CreateUIGameObjectInto(this);

    UIVerticalLayout *mainVL = AddComponent<UIVerticalLayout>();

    UIImageRenderer *bg = AddComponent<UIImageRenderer>();
    bg->SetTint(Color::Blue());

    p_scrollPanel = GameObjectFactory::CreateUIScrollPanel();
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

    CreateRow();
}

UIEffectLayers::~UIEffectLayers()
{
}

UIEffectLayerRow *UIEffectLayers::CreateRow()
{
    UIEffectLayerRow *effectLayerRow = new UIEffectLayerRow();

    effectLayerRow->SetParent(p_scrollPanelContainer);

    p_effectLayerRows.PushBack(effectLayerRow);
    return effectLayerRow;
}
