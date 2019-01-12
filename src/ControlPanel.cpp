#include "ControlPanel.h"

#include "Bang/Dialog.h"
#include "Bang/Extensions.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Input.h"
#include "Bang/UIButton.h"
#include "Bang/UIComboBox.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UIInputNumber.h"
#include "Bang/UILabel.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UISlider.h"
#include "Bang/UITextRenderer.h"
#include "Bang/UIToolButton.h"
#include "Bang/UIVerticalLayout.h"
#include "BangEditor/EditorDialog.h"
#include "BangEditor/EditorPaths.h"

#include "MainScene.h"
#include "UIEffectLayerRow.h"
#include "UIEffectLayers.h"
#include "View3DScene.h"

using namespace Bang;
using namespace BangEditor;

ControlPanel::ControlPanel()
{
    GameObjectFactory::CreateUIGameObjectInto(this);

    UIVerticalLayout *vl = AddComponent<UIVerticalLayout>();
    vl->SetPaddings(10);

    GameObjectFactory::AddOuterBorder(this, Vector2i(3), Color::Black());

    UILayoutElement *le = AddComponent<UILayoutElement>();
    le->SetMinWidth(300);

    UIImageRenderer *bg = AddComponent<UIImageRenderer>();
    bg->SetTint(Color::Gray());

    auto CreateRow = [](const String &labelStr = "", GameObject *go = nullptr) {
        GameObject *rowGo = GameObjectFactory::CreateUIGameObject();
        UIHorizontalLayout *rowHL = rowGo->AddComponent<UIHorizontalLayout>();
        rowHL->SetSpacing(10);

        if (!labelStr.IsEmpty())
        {
            UILabel *uiLabel = GameObjectFactory::CreateUILabel();
            uiLabel->GetText()->SetContent(labelStr);
            uiLabel->GetText()->SetHorizontalAlign(HorizontalAlignment::LEFT);
            uiLabel->GetGameObject()->SetParent(rowGo);
        }

        if (go)
        {
            UILayoutElement *goLE = go->AddComponent<UILayoutElement>();
            goLE->SetFlexibleWidth(9999.9f);
            go->SetParent(rowGo);
        }

        return rowGo;
    };

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

        p_openModelButton = GameObjectFactory::CreateUIButton("Open");
        p_openModelButton->AddClickedCallback([this]() { OpenModel(); });
        p_openModelButton->GetGameObject()->SetParent(buttonsRow);

        p_exportModelButton = GameObjectFactory::CreateUIButton("Export");
        p_exportModelButton->AddClickedCallback([this]() { ExportModel(); });
        p_exportModelButton->GetGameObject()->SetParent(buttonsRow);
    }

    // View buttons row
    {
        GameObject *sceneModeRow = GameObjectFactory::CreateUIGameObject();
        UIHorizontalLayout *hl =
            sceneModeRow->AddComponent<UIHorizontalLayout>();
        hl->SetSpacing(10);
        UILayoutElement *rowLE = sceneModeRow->AddComponent<UILayoutElement>();
        rowLE->SetMinHeight(20);
        sceneModeRow->SetParent(this);

        GameObjectFactory::CreateUIHSeparator(LayoutSizeType::MIN, 15)
            ->SetParent(this);

        UILabel *label = GameObjectFactory::CreateUILabel();
        label->GetText()->SetContent("Scene mode:");
        label->GetGameObject()->SetParent(sceneModeRow);

        p_sceneModeComboBox = GameObjectFactory::CreateUIComboBox();
        p_sceneModeComboBox->AddItem("View 3D",
                                     SCAST<uint>(MainScene::SceneMode::VIEW3D));
        p_sceneModeComboBox->AddItem("Uv",
                                     SCAST<uint>(MainScene::SceneMode::UV));
        p_sceneModeComboBox->AddItem(
            "Textures", SCAST<uint>(MainScene::SceneMode::TEXTURES));

        p_sceneModeComboBox->GetGameObject()->SetParent(sceneModeRow);

        GameObjectFactory::CreateUIHSpacer(LayoutSizeType::FLEXIBLE, 9999.0f)
            ->SetParent(sceneModeRow);
    }

    // Effect layers
    {
        CreateRow("Effect layers")->SetParent(this);

        p_effectLayers = new UIEffectLayers();
        UILayoutElement *le = p_effectLayers->AddComponent<UILayoutElement>();
        le->SetMinHeight(60);
        le->SetPreferredHeight(200);
        le->SetFlexibleWidth(1.0f);
        p_effectLayers->SetParent(this);

        GameObjectFactory::CreateUIHSeparator(LayoutSizeType::MIN, 15.0f)
            ->SetParent(this);
    }

    // Dirt showgroup
    {
        CreateRow("Dirt")->SetParent(this);
        p_dirtSeedInput = GameObjectFactory::CreateUIInputNumber();
        p_dirtSeedInput->SetMinValue(0);
        p_dirtSeedInput->SetDecimalPlaces(0);
        p_dirtSeedInput->EventEmitter<IEventsValueChanged>::RegisterListener(
            this);
        p_dirtSeedInput->SetValue(0);

        p_dirtFrequencyMultiplyInput = GameObjectFactory::CreateUISlider();
        p_dirtFrequencyMultiplyInput->SetMinMaxValues(0.0f, 4.0f);
        p_dirtFrequencyMultiplyInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);
        p_dirtFrequencyMultiplyInput->SetValue(2.5f);

        p_dirtFrequencyInput = GameObjectFactory::CreateUISlider();
        p_dirtFrequencyInput->SetMinMaxValues(0.0f, 5.0f);
        p_dirtFrequencyInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);
        p_dirtFrequencyInput->SetValue(4.0f);

        p_dirtAmplitudeInput = GameObjectFactory::CreateUISlider();
        p_dirtAmplitudeInput->SetMinMaxValues(0.0f, 2.0f);
        p_dirtAmplitudeInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);
        p_dirtAmplitudeInput->SetValue(0.6f);

        p_dirtAmplitudeMultiplyInput = GameObjectFactory::CreateUISlider();
        p_dirtAmplitudeMultiplyInput->SetMinMaxValues(0.0f, 1.0f);
        p_dirtAmplitudeMultiplyInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);
        p_dirtAmplitudeMultiplyInput->SetValue(0.6f);

        CreateRow("Intensity", p_dirtAmplitudeInput->GetGameObject())
            ->SetParent(this);
        CreateRow("Stains size", p_dirtFrequencyInput->GetGameObject())
            ->SetParent(this);
        CreateRow("Grain", p_dirtFrequencyMultiplyInput->GetGameObject())
            ->SetParent(this);
        CreateRow("Splash", p_dirtAmplitudeMultiplyInput->GetGameObject())
            ->SetParent(this);
        CreateRow("Seed", p_dirtSeedInput->GetGameObject())->SetParent(this);
    }
}

ControlPanel::~ControlPanel()
{
}

void ControlPanel::Update()
{
    GameObject::Update();

    if (Input::GetKeyDown(Key::O))
    {
        OpenModel();
    }
    else if (Input::GetKeyDown(Key::E))
    {
        ExportModel();
    }

    MainScene::GetInstance()->SetSceneMode(
        SCAST<MainScene::SceneMode>(p_sceneModeComboBox->GetSelectedValue()));

    p_exportModelButton->SetBlocked(!(GetOpenModelPath().IsFile()));
}

void ControlPanel::OpenModel()
{
    Path modelPath = Dialog::OpenFilePath(
        "Open model", Extensions::GetModelExtensions(), GetInitialDir());
    OpenModel(modelPath);
}

void ControlPanel::OpenModel(const Path &modelPath)
{
    if (modelPath.IsFile())
    {
        m_openModelPath = modelPath;
        MainScene::GetInstance()->LoadModel(modelPath);
    }
}

void ControlPanel::ExportModel()
{
    const String extension = "dae";
    const Path exportedModelPath = Dialog::SaveFilePath(
        "Export model",
        extension,
        GetInitialDir(),
        GetOpenModelPath().GetName() + String(".") + extension);

    ModelIO::ExportModel(
        MainScene::GetInstance()->GetView3DScene()->GetModelGameObject(),
        exportedModelPath);
}

void ControlPanel::CreateNewEffectLayer()
{
    View3DScene *view3DScene = MainScene::GetInstance()->GetView3DScene();
    view3DScene->CreateNewEffectLayer();
    p_effectLayers->CreateNewEffectLayerRow();
    view3DScene->InvalidateEffectLayersTextures();
}

uint ControlPanel::GetDirtSeed() const
{
    return SCAST<uint>(p_dirtSeedInput->GetValue());
}

float ControlPanel::GetDirtOctaves() const
{
    return 4.0f;
}

float ControlPanel::GetDirtFrequency() const
{
    return (p_dirtFrequencyInput->GetInputNumber()->GetMaxValue() -
            p_dirtFrequencyInput->GetValue());
}

float ControlPanel::GetDirtAmplitude() const
{
    return p_dirtAmplitudeInput->GetValue();
}

float ControlPanel::GetDirtFrequencyMultiply() const
{
    return p_dirtFrequencyMultiplyInput->GetValue();
}

float ControlPanel::GetDirtAmplitudeMultiply() const
{
    return p_dirtAmplitudeMultiplyInput->GetValue();
}

void ControlPanel::SetSceneModeOnComboBox(MainScene::SceneMode sceneMode)
{
    p_sceneModeComboBox->SetSelectionByValue(SCAST<uint>(sceneMode));
}

Path ControlPanel::GetInitialDir() const
{
    return EditorPaths::GetExecutableDir();
}

Path ControlPanel::GetOpenModelPath() const
{
    return m_openModelPath;
}

void ControlPanel::OnValueChanged(EventEmitter<IEventsValueChanged> *ee)
{
    if (ee == p_dirtSeedInput || ee == p_dirtFrequencyInput ||
        ee == p_dirtAmplitudeInput || ee == p_dirtAmplitudeMultiplyInput ||
        ee == p_dirtFrequencyMultiplyInput)
    {
        MainScene::GetInstance()
            ->GetView3DScene()
            ->InvalidateEffectLayersTextures();
    }
}
