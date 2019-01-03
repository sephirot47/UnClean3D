#include "ControlPanel.h"

#include "Bang/Dialog.h"
#include "Bang/Extensions.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/UIButton.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UIVerticalLayout.h"
#include "BangEditor/EditorDialog.h"
#include "BangEditor/EditorPaths.h"
#include "EditScene.h"

using namespace Bang;
using namespace BangEditor;

ControlPanel::ControlPanel()
{
    GameObjectFactory::CreateUIGameObjectInto(this);

    UIVerticalLayout *vl = AddComponent<UIVerticalLayout>();
    vl->SetPaddings(4);

    GameObjectFactory::AddOuterBorder(this, Vector2i(3), Color::Black());

    UILayoutElement *le = AddComponent<UILayoutElement>();
    le->SetMinWidth(300);

    UIImageRenderer *bg = AddComponent<UIImageRenderer>();
    bg->SetTint(Color::Gray());

    // Open/Export buttons row
    {
        GameObject *buttonsRow = GameObjectFactory::CreateUIGameObject();
        UIHorizontalLayout *hl = buttonsRow->AddComponent<UIHorizontalLayout>();
        hl->SetSpacing(10);
        UILayoutElement *rowLE = buttonsRow->AddComponent<UILayoutElement>();
        rowLE->SetMinHeight(20);
        buttonsRow->SetParent(this);

        GameObjectFactory::CreateUIHSeparator(LayoutSizeType::MIN, 15)
            ->SetParent(this);

        UIButton *openModelButton = GameObjectFactory::CreateUIButton("Open");
        openModelButton->AddClickedCallback([this]() { OpenModel(); });
        openModelButton->GetGameObject()->SetParent(buttonsRow);

        UIButton *exportModelButton =
            GameObjectFactory::CreateUIButton("Export");
        exportModelButton->AddClickedCallback([this]() { ExportModel(); });
        exportModelButton->GetGameObject()->SetParent(buttonsRow);
    }
}

ControlPanel::~ControlPanel()
{
}

void ControlPanel::OpenModel()
{
    Path modelPath = Dialog::OpenFilePath(
        "Open model", Extensions::GetModelExtensions(), EditorPaths::GetHome());
    if (modelPath.IsFile())
    {
        p_editScene->OpenModel(modelPath);
    }
}

void ControlPanel::ExportModel()
{
    Dialog::OpenFilePath("Export model",
                         Extensions::GetModelExtensions(),
                         EditorPaths::GetHome());
}

void ControlPanel::SetEditScene(EditScene *editScene)
{
    p_editScene = editScene;
}
