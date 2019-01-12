#include "UIEffectLayerRow.h"

#include "Bang/GameObjectFactory.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILabel.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UITextRenderer.h"
#include "Bang/UIVerticalLayout.h"

#include "Bang/Random.h"

using namespace Bang;

UIEffectLayerRow::UIEffectLayerRow()
{
    GameObjectFactory::CreateUIGameObjectInto(this);

    UIImageRenderer *bg = AddComponent<UIImageRenderer>();
    bg->SetTint(Random::GetColorOpaque());

    UILayoutElement *le = AddComponent<UILayoutElement>();
    le->SetFlexibleWidth(1.0f);
    le->SetMinHeight(40);
}

UIEffectLayerRow::~UIEffectLayerRow()
{
}
