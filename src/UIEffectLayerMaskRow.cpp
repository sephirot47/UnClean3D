#include "UIEffectLayerMaskRow.h"

#include "Bang/GameObjectFactory.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILabel.h"
#include "Bang/UILayoutElement.h"

using namespace Bang;

UIEffectLayerMaskRow::UIEffectLayerMaskRow()
{
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

    GameObject *innerHLGo = GameObjectFactory::CreateUIGameObject();

    UILayoutElement *le = innerHLGo->AddComponent<UILayoutElement>();
    le->SetFlexibleWidth(1.0f);

    UIHorizontalLayout *hl = innerHLGo->AddComponent<UIHorizontalLayout>();
    hl->SetPaddings(10);
    hl->SetPaddingLeft(40);

    UILabel *label = GameObjectFactory::CreateUILabel();
    label->GetText()->SetContent("Mask");
    label->GetText()->SetHorizontalAlign(HorizontalAlignment::LEFT);
    label->GetGameObject()->SetParent(innerHLGo);

    GameObjectFactory::AddInnerBorder(
        innerHLGo, Vector2i(1), Color::White().WithValue(0.3f));
    innerHLGo->SetParent(this);
}

UIEffectLayerMaskRow::~UIEffectLayerMaskRow()
{
}
