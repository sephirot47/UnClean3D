#include "BrushInspectorWidget.h"

#include "Bang/Assets.h"
#include "Bang/Extensions.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Path.h"
#include "Bang/Paths.h"
#include "Bang/RectTransform.h"
#include "Bang/UIContentSizeFitter.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILabel.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UIScrollArea.h"
#include "Bang/UIScrollPanel.h"
#include "Bang/UIVerticalLayout.h"

#include "ControlPanel.h"
#include "TextureContainer.h"

using namespace Bang;

BrushInspectorWidget::BrushInspectorWidget()
{
}

BrushInspectorWidget::~BrushInspectorWidget()
{
}

void BrushInspectorWidget::InitInnerWidgets()
{
    SerializableInspectorWidget::InitInnerWidgets();

    p_brushesImagesContainer = GameObjectFactory::CreateUIGameObject();
    p_brushesImagesContainer->AddComponent<UIVerticalLayout>();
    p_brushesImagesContainer->GetRectTransform()->SetPivotPosition(
        Vector2(-1, 1));

    auto csf = p_brushesImagesContainer->AddComponent<UIContentSizeFitter>();
    csf->SetVerticalSizeType(LayoutSizeType::PREFERRED);
    csf->SetHorizontalSizeType(LayoutSizeType::PREFERRED);

    UIScrollPanel *scrollPanel = GameObjectFactory::CreateUIScrollPanel();
    scrollPanel->SetVerticalScrollEnabled(false);
    scrollPanel->SetHorizontalScrollEnabled(true);
    scrollPanel->GetScrollArea()->SetContainedGameObject(
        p_brushesImagesContainer);

    GameObject *brushesImagesRows = GameObjectFactory::CreateUIGameObject();
    auto hl = brushesImagesRows->AddComponent<UIHorizontalLayout>();
    hl->SetSpacing(10);
    hl->SetPaddings(10);
    brushesImagesRows->SetParent(p_brushesImagesContainer);

    UILayoutElement *rowLE = brushesImagesRows->AddComponent<UILayoutElement>();
    rowLE->SetMinHeight(100);

    Path brushesPath = Paths::GetProjectAssetsDir().Append("Brushes");
    Array<Path> brushesPaths = brushesPath.GetFiles(
        FindFlag::SIMPLE, Extensions::GetImageExtensions());

    for (const Path &brushPath : brushesPaths)
    {
        TextureContainer *texCont = new TextureContainer();
        texCont->SetCanBeFocused(true);
        texCont->GetLabel()->GetGameObject()->SetEnabled(false);
        texCont->GetImageRenderer()->SetImageTexture(brushPath);
        texCont->GetImageLayoutElement()->SetMinSize(Vector2i(64));
        texCont->GetImageLayoutElement()->SetPreferredSize(Vector2i(64));
        texCont->GetImageLayoutElement()->SetFlexibleSize(Vector2::Zero());
        texCont->GetFocusable()->AddEventCallback(
            [this, texCont](UIFocusable *, const UIEvent &event) {
                if (event.type == UIEvent::Type::MOUSE_CLICK_FULL)
                {
                    ControlPanel::GetInstance()->SetBrushTexture(
                        texCont->GetImageRenderer()->GetImageTexture());
                }
                return UIEventResult::IGNORE;
            });
        texCont->SetParent(brushesImagesRows);

        p_textureContainers.PushBack(texCont);
    }

    AddWidget(scrollPanel->GetGameObject(), 120);
}

void BrushInspectorWidget::UpdateFromReference()
{
}
