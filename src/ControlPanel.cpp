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
#include "BangEditor/UIInputColor.h"

#include "EffectLayer.h"
#include "EffectLayerDirt.h"
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
        p_uiEffectLayers = new UIEffectLayers();
        UILayoutElement *le = p_uiEffectLayers->AddComponent<UILayoutElement>();
        le->SetMinHeight(60);
        le->SetPreferredHeight(200);
        le->SetFlexibleWidth(1.0f);
        p_uiEffectLayers->SetParent(this);

        GameObjectFactory::CreateUIHSeparator(LayoutSizeType::MIN, 15.0f)
            ->SetParent(this);
    }

    // Dirt showgroup
    p_dirtParamsGo = GameObjectFactory::CreateUIGameObject();
    {
        UIVerticalLayout *vl = p_dirtParamsGo->AddComponent<UIVerticalLayout>();
        vl->SetSpacing(5);

        UILabel *dirtLabel = GameObjectFactory::CreateUILabel();
        dirtLabel->GetText()->SetContent("Dirt");
        dirtLabel->GetText()->SetTextSize(14);
        dirtLabel->GetText()->SetHorizontalAlign(HorizontalAlignment::LEFT);
        dirtLabel->GetGameObject()->SetParent(p_dirtParamsGo);

        p_dirtSeedInput = GameObjectFactory::CreateUIInputNumber();
        p_dirtSeedInput->SetMinValue(0);
        p_dirtSeedInput->SetDecimalPlaces(0);
        p_dirtSeedInput->EventEmitter<IEventsValueChanged>::RegisterListener(
            this);

        p_dirtColor0Input = new UIInputColor();
        p_dirtColor0Input->EventEmitter<IEventsValueChanged>::RegisterListener(
            this);
        p_dirtColor1Input = new UIInputColor();
        p_dirtColor1Input->EventEmitter<IEventsValueChanged>::RegisterListener(
            this);

        p_dirtFrequencyMultiplyInput = GameObjectFactory::CreateUISlider();
        p_dirtFrequencyMultiplyInput->SetMinMaxValues(0.0f, 4.0f);
        p_dirtFrequencyMultiplyInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);

        p_dirtFrequencyInput = GameObjectFactory::CreateUISlider();
        p_dirtFrequencyInput->SetMinMaxValues(0.0f, 5.0f);
        p_dirtFrequencyInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);

        p_dirtAmplitudeInput = GameObjectFactory::CreateUISlider();
        p_dirtAmplitudeInput->SetMinMaxValues(0.0f, 2.0f);
        p_dirtAmplitudeInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);

        p_dirtAmplitudeMultiplyInput = GameObjectFactory::CreateUISlider();
        p_dirtAmplitudeMultiplyInput->SetMinMaxValues(0.0f, 1.0f);
        p_dirtAmplitudeMultiplyInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);

        CreateRow("Intensity", p_dirtAmplitudeInput->GetGameObject())
            ->SetParent(p_dirtParamsGo);
        CreateRow("Stains size", p_dirtFrequencyInput->GetGameObject())
            ->SetParent(p_dirtParamsGo);
        CreateRow("Grain", p_dirtFrequencyMultiplyInput->GetGameObject())
            ->SetParent(p_dirtParamsGo);
        CreateRow("Sharpness", p_dirtAmplitudeMultiplyInput->GetGameObject())
            ->SetParent(p_dirtParamsGo);
        CreateRow("Seed", p_dirtSeedInput->GetGameObject())
            ->SetParent(p_dirtParamsGo);
        CreateRow("Color0", p_dirtColor0Input)->SetParent(p_dirtParamsGo);
        CreateRow("Color1", p_dirtColor1Input)->SetParent(p_dirtParamsGo);

        p_dirtParamsGo->SetParent(this);
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

    p_dirtParamsGo->SetEnabled(false);

    Array<EffectLayer *> selectedEffectLayers =
        GetView3DScene()->GetSelectedEffectLayers();
    if (selectedEffectLayers.Size() >= 1)
    {
        EffectLayer *selectedEffectLayer = selectedEffectLayers.Front();
        if (EffectLayerImplementation *impl =
                selectedEffectLayer->GetImplementation())
        {
            p_dirtParamsGo->SetEnabled(DCAST<EffectLayerDirt *>(impl) !=
                                       nullptr);
        }
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

    GetView3DScene()->ApplyCompositeTexturesToModel();
    ModelIO::ExportModel(GetView3DScene()->GetModelGameObject(),
                         exportedModelPath);
    GetView3DScene()->RestoreOriginalAlbedoTexturesToModel();
}

void ControlPanel::CreateNewEffectLayer()
{
    GetView3DScene()->CreateNewEffectLayer();
    p_uiEffectLayers->CreateNewEffectLayerRow();
}

void ControlPanel::RemoveEffectLayer(uint effectLayerIdx)
{
    GetView3DScene()->RemoveEffectLayer(effectLayerIdx);
    p_uiEffectLayers->RemoveEffectLayer(effectLayerIdx);
}

void ControlPanel::UpdateSelectedEffectLayerParameters()
{
    m_params.dirtSeed = p_dirtSeedInput->GetValue();
    m_params.dirtFrequency = p_dirtFrequencyInput->GetValue();
    m_params.dirtAmplitude = p_dirtAmplitudeInput->GetValue();
    m_params.dirtFrequencyMultiply = p_dirtFrequencyMultiplyInput->GetValue();
    m_params.dirtAmplitudeMultiply = p_dirtAmplitudeMultiplyInput->GetValue();
    m_params.dirtColor0 = p_dirtColor0Input->GetColor();
    m_params.dirtColor1 = p_dirtColor1Input->GetColor();

    MainScene::GetInstance()->GetView3DScene()->UpdateParameters(
        GetParameters());
}

void ControlPanel::UpdateInputsAndParametersFromSelectedEffectLayer()
{
    Array<EffectLayer *> selectedEffectLayers =
        GetView3DScene()->GetSelectedEffectLayers();
    if (selectedEffectLayers.Size() >= 1)
    {
        EffectLayer *selectedEffectLayer = selectedEffectLayers.Front();
        m_params = selectedEffectLayer->GetParameters();
    }

    EventListener<IEventsValueChanged>::SetReceiveEvents(false);

    p_dirtSeedInput->SetValue(GetParameters().dirtSeed);
    p_dirtFrequencyInput->SetValue(GetParameters().dirtFrequency);
    p_dirtFrequencyMultiplyInput->SetValue(
        GetParameters().dirtFrequencyMultiply);
    p_dirtAmplitudeInput->SetValue(GetParameters().dirtAmplitude);
    p_dirtAmplitudeMultiplyInput->SetValue(
        GetParameters().dirtAmplitudeMultiply);
    p_dirtColor0Input->SetColor(GetParameters().dirtColor0);
    p_dirtColor1Input->SetColor(GetParameters().dirtColor1);

    EventListener<IEventsValueChanged>::SetReceiveEvents(true);
}

uint ControlPanel::GetSelectedUIEffectLayerIndex() const
{
    return p_uiEffectLayers->GetSelectedEffectLayerRowIndex();
}

bool ControlPanel::IsVisibleUIEffectLayer(uint effectLayerIdx) const
{
    if (effectLayerIdx < p_uiEffectLayers->GetUIEffectLayerRows().Size())
    {
        UIEffectLayerRow *uiEffectLayerRow =
            p_uiEffectLayers->GetUIEffectLayerRows()[effectLayerIdx];
        return uiEffectLayerRow->GetIsLayerVisible();
    }
    return false;
}

const EffectLayerParameters &ControlPanel::GetParameters() const
{
    return m_params;
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

View3DScene *ControlPanel::GetView3DScene() const
{
    return MainScene::GetInstance()->GetView3DScene();
}

void ControlPanel::OnValueChanged(EventEmitter<IEventsValueChanged> *)
{
    UpdateSelectedEffectLayerParameters();
}
