#include "ControlPanel.h"

#include "Bang/Dialog.h"
#include "Bang/Extensions.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Input.h"
#include "Bang/Transform.h"
#include "Bang/UIButton.h"
#include "Bang/UICheckBox.h"
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
#include "BangEditor/SerializableInspectorWidget.h"
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
    le->SetMinWidth(600);

    UIImageRenderer *bg = AddComponent<UIImageRenderer>();
    bg->SetTint(Color::Gray());

    auto CreateRow = [](const String &labelStr = "",
                        GameObject *go = nullptr,
                        bool stretch = true) {
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
            if (stretch)
            {
                goLE->SetFlexibleWidth(9999.9f);
            }
            go->SetParent(rowGo);
        }

        return rowGo;
    };

    // Open/Export buttons row
    {
        UILabel *fileSettingsLabel = GameObjectFactory::CreateUILabel();
        fileSettingsLabel->GetText()->SetContent("File settings");
        fileSettingsLabel->GetText()->SetTextSize(14);
        fileSettingsLabel->GetText()->SetHorizontalAlign(
            HorizontalAlignment::LEFT);
        CreateRow("", fileSettingsLabel->GetGameObject())->SetParent(this);

        GameObject *buttonsRow = GameObjectFactory::CreateUIGameObject();
        UIHorizontalLayout *hl = buttonsRow->AddComponent<UIHorizontalLayout>();
        hl->SetSpacing(10);
        UILayoutElement *rowLE = buttonsRow->AddComponent<UILayoutElement>();
        rowLE->SetMinHeight(20);
        buttonsRow->SetParent(this);

        p_openModelButton = GameObjectFactory::CreateUIButton("Open");
        p_openModelButton->AddClickedCallback([this]() { OpenModel(); });
        p_openModelButton->GetGameObject()->SetParent(buttonsRow);

        p_exportModelButton = GameObjectFactory::CreateUIButton("Export");
        p_exportModelButton->AddClickedCallback([this]() { ExportModel(); });
        p_exportModelButton->GetGameObject()->SetParent(buttonsRow);

        GameObjectFactory::CreateUIHSeparator(LayoutSizeType::MIN, 15)
            ->SetParent(this);
    }

    // View buttons row
    {
        UILabel *viewSettingsLabel = GameObjectFactory::CreateUILabel();
        viewSettingsLabel->GetText()->SetContent("View settings");
        viewSettingsLabel->GetText()->SetTextSize(14);
        viewSettingsLabel->GetText()->SetHorizontalAlign(
            HorizontalAlignment::LEFT);
        CreateRow("", viewSettingsLabel->GetGameObject())->SetParent(this);

        GameObject *sceneModeRow = GameObjectFactory::CreateUIGameObject();
        UIHorizontalLayout *hl =
            sceneModeRow->AddComponent<UIHorizontalLayout>();
        hl->SetSpacing(10);
        UILayoutElement *rowLE = sceneModeRow->AddComponent<UILayoutElement>();
        rowLE->SetMinHeight(20);
        sceneModeRow->SetParent(this);

        UILabel *label = GameObjectFactory::CreateUILabel();
        label->GetText()->SetContent("Scene mode:");
        label->GetGameObject()->SetParent(sceneModeRow);

        p_sceneModeComboBox = GameObjectFactory::CreateUIComboBox();
        p_sceneModeComboBox->AddItem("View 3D",
                                     SCAST<uint>(MainScene::SceneMode::VIEW3D));
        p_sceneModeComboBox->AddItem(
            "Textures", SCAST<uint>(MainScene::SceneMode::TEXTURES));

        p_sceneModeComboBox->GetGameObject()->SetParent(sceneModeRow);

        GameObjectFactory::CreateUIHSpacer(LayoutSizeType::FLEXIBLE, 9999.0f)
            ->SetParent(sceneModeRow);
        GameObjectFactory::CreateUIHSeparator(LayoutSizeType::MIN, 30.0f)
            ->SetParent(this);
    }

    // General settings
    {
        UILabel *generalSettingsLabel = GameObjectFactory::CreateUILabel();
        generalSettingsLabel->GetText()->SetContent("General settings");
        generalSettingsLabel->GetText()->SetTextSize(14);
        generalSettingsLabel->GetText()->SetHorizontalAlign(
            HorizontalAlignment::LEFT);
        CreateRow("", generalSettingsLabel->GetGameObject())->SetParent(this);

        p_baseRoughnessInput = GameObjectFactory::CreateUISlider(0, 1);
        p_baseRoughnessInput->SetValue(1.0f);
        p_baseRoughnessInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);
        CreateRow("Base roughness", p_baseRoughnessInput->GetGameObject())
            ->SetParent(this);

        p_baseMetalnessInput = GameObjectFactory::CreateUISlider(0, 1);
        p_baseMetalnessInput->SetValue(0.0f);
        p_baseMetalnessInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);
        CreateRow("Base metalness", p_baseMetalnessInput->GetGameObject())
            ->SetParent(this);

        p_texturesSizeInput = GameObjectFactory::CreateUIComboBox();
        p_texturesSizeInput->AddItem("64", 64);
        p_texturesSizeInput->AddItem("128", 128);
        p_texturesSizeInput->AddItem("256", 256);
        p_texturesSizeInput->AddItem("512", 512);
        p_texturesSizeInput->AddItem("1024", 1024);
        p_texturesSizeInput->AddItem("2048", 2048);
        p_texturesSizeInput->AddItem("4096", 4096);
        p_texturesSizeInput->SetSelectionByIndex(4);
        p_texturesSizeInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);
        CreateRow("Textures size", p_texturesSizeInput->GetGameObject(), false)
            ->SetParent(this);

        GameObjectFactory::CreateUIHSeparator(LayoutSizeType::MIN, 30.0f)
            ->SetParent(this);
    }

    // Effect layers
    {
        p_uiEffectLayers = new UIEffectLayers();
        UILayoutElement *le = p_uiEffectLayers->AddComponent<UILayoutElement>();
        le->SetMinHeight(60);
        le->SetPreferredHeight(200);
        le->SetFlexibleWidth(1.0f);
        p_uiEffectLayers->SetParent(this);

        GameObjectFactory::CreateUIHSeparator(LayoutSizeType::MIN, 30.0f)
            ->SetParent(this);
    }

    // Mask
    {
        UILabel *maskLabel = GameObjectFactory::CreateUILabel();
        maskLabel->GetText()->SetContent("Mask");
        maskLabel->GetText()->SetTextSize(14);
        maskLabel->GetText()->SetHorizontalAlign(HorizontalAlignment::LEFT);
        CreateRow("", maskLabel->GetGameObject())->SetParent(this);

        p_maskBrushEnabledButton =
            GameObjectFactory::CreateUIToolButton("Draw Mask (M)");
        p_maskBrushEnabledButton->SetOn(false);
        p_maskBrushEnabledButton->GetGameObject()->SetParent(this);

        p_maskBrushSizeInput = GameObjectFactory::CreateUISlider(1, 500);
        p_maskBrushSizeInput->SetValue(50.0f);
        p_maskBrushSizeInputRow =
            CreateRow("Brush size", p_maskBrushSizeInput->GetGameObject());
        p_maskBrushSizeInputRow->SetParent(this);

        p_maskBrushHardnessInput = GameObjectFactory::CreateUISlider(0, 1);
        p_maskBrushHardnessInput->SetValue(0.1f);
        p_maskBrushHardnessInputRow = CreateRow(
            "Brush hardness", p_maskBrushHardnessInput->GetGameObject());
        p_maskBrushHardnessInputRow->SetParent(this);

        p_maskBrushStrengthInput = GameObjectFactory::CreateUISlider(0, 1);
        p_maskBrushStrengthInput->SetValue(1.0f);
        p_maskBrushStrengthInputRow = CreateRow(
            "Brush strength", p_maskBrushStrengthInput->GetGameObject());
        p_maskBrushStrengthInputRow->SetParent(this);

        p_seeMaskButton =
            GameObjectFactory::CreateUIToolButton("See Mask (Shift + M)");
        p_seeMaskButton->SetOn(false);
        p_seeMaskButton->GetGameObject()->SetParent(this);

        p_maskBrushDepthAwareButton =
            GameObjectFactory::CreateUIToolButton("Depth aware mask brush");
        p_maskBrushDepthAwareButton->SetOn(true);
        p_maskBrushDepthAwareButton->GetGameObject()->SetParent(this);

        p_eraseMaskButton =
            GameObjectFactory::CreateUIToolButton("Erase Mask (Ctrl)");
        p_eraseMaskButton->SetOn(false);
        p_eraseMaskButton->GetGameObject()->SetParent(this);

        p_clearMaskButton = GameObjectFactory::CreateUIButton("Clear Mask (C)");
        p_clearMaskButton->AddClickedCallback([this]() { ClearMask(); });
        p_clearMaskButton->GetGameObject()->SetParent(this);

        GameObjectFactory::CreateUIHSeparator(LayoutSizeType::MIN, 30.0f)
            ->SetParent(this);
    }

    p_serializableWidgetContainer = GameObjectFactory::CreateUIGameObject();
    {
        p_serializableWidgetContainer->AddComponent<UIVerticalLayout>();

        p_serializableWidget = new SerializableInspectorWidget();
        p_serializableWidget->Init();
        p_serializableWidget->SetParent(p_serializableWidgetContainer);

        p_serializableWidgetContainer->SetParent(this);
    }

    // Dirt
    p_dirtParamsGo = GameObjectFactory::CreateUIGameObject();
    {
        UIVerticalLayout *vl = p_dirtParamsGo->AddComponent<UIVerticalLayout>();
        vl->SetSpacing(5);

        UILabel *label = GameObjectFactory::CreateUILabel();
        label->GetText()->SetContent("Dirt");
        label->GetText()->SetTextSize(14);
        label->GetText()->SetHorizontalAlign(HorizontalAlignment::LEFT);
        CreateRow("", label->GetGameObject())->SetParent(p_dirtParamsGo);

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

        p_dirtFrequencyMultiplyInput = GameObjectFactory::CreateUISlider(0, 4);
        p_dirtFrequencyMultiplyInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);

        p_dirtFrequencyInput = GameObjectFactory::CreateUISlider(0, 25, 1);
        p_dirtFrequencyInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);

        p_dirtAmplitudeInput = GameObjectFactory::CreateUISlider(0, 2);
        p_dirtAmplitudeInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);

        p_dirtAmplitudeMultiplyInput = GameObjectFactory::CreateUISlider(0, 1);
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

    // Normal Lines
    p_normalLinesParamsGo = GameObjectFactory::CreateUIGameObject();
    {
        UIVerticalLayout *vl =
            p_normalLinesParamsGo->AddComponent<UIVerticalLayout>();
        vl->SetSpacing(5);

        UILabel *label = GameObjectFactory::CreateUILabel();
        label->GetText()->SetContent("Normal Lines");
        label->GetText()->SetTextSize(14);
        label->GetText()->SetHorizontalAlign(HorizontalAlignment::LEFT);
        CreateRow("", label->GetGameObject())->SetParent(p_normalLinesParamsGo);

        p_normalLinesHeightInput = GameObjectFactory::CreateUISlider(0, 1);
        p_normalLinesHeightInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);
        p_normalLinesWidthInput = GameObjectFactory::CreateUISlider(0.01f, 30);
        p_normalLinesWidthInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);

        CreateRow("Height", p_normalLinesHeightInput->GetGameObject())
            ->SetParent(p_normalLinesParamsGo);
        CreateRow("Width", p_normalLinesWidthInput->GetGameObject())
            ->SetParent(p_normalLinesParamsGo);
        p_normalLinesParamsGo->SetParent(this);
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
    else if (Input::GetKeyDown(Key::M) && !Input::GetKey(Key::LSHIFT))
    {
        p_maskBrushEnabledButton->SetOn(!p_maskBrushEnabledButton->GetOn());
    }
    else if (Input::GetKeyDown(Key::C))
    {
        ClearMask();
    }

    if (GetMaskBrushEnabled())
    {
        if (Input::GetKey(Key::LSHIFT))
        {
            float maskBrushSizeIncrement = Input::GetMouseWheel().y;
            maskBrushSizeIncrement *= (p_maskBrushSizeInput->GetValue() / 10);
            p_maskBrushSizeInput->SetValue(p_maskBrushSizeInput->GetValue() +
                                           maskBrushSizeIncrement);

            if (Input::GetKeyDown(Key::M))
            {
                p_seeMaskButton->SetOn(!p_seeMaskButton->GetOn());
            }
        }
    }
    p_eraseMaskButton->SetOn(GetMaskBrushEnabled() &&
                             Input::GetKey(Key::LCTRL));

    p_maskBrushEnabledButton->SetBlocked(
        GetView3DScene()->GetSelectedEffectLayers().Size() == 0);
    p_maskBrushDepthAwareButton->GetGameObject()->SetEnabled(
        GetMaskBrushEnabled());
    p_eraseMaskButton->GetGameObject()->SetEnabled(GetMaskBrushEnabled());
    p_seeMaskButton->GetGameObject()->SetEnabled(GetMaskBrushEnabled());
    p_clearMaskButton->GetGameObject()->SetEnabled(GetMaskBrushEnabled());
    p_maskBrushStrengthInputRow->SetEnabled(GetMaskBrushEnabled());
    p_maskBrushSizeInputRow->SetEnabled(GetMaskBrushEnabled());
    p_maskBrushHardnessInputRow->SetEnabled(GetMaskBrushEnabled());

    p_dirtParamsGo->SetEnabled(false);
    p_normalLinesParamsGo->SetEnabled(false);

    Array<EffectLayer *> selectedEffectLayers =
        GetView3DScene()->GetSelectedEffectLayers();
    if (selectedEffectLayers.Size() >= 1)
    {
        EffectLayer *selectedEffectLayer = selectedEffectLayers.Front();

        if (EffectLayerImplementation *impl =
                selectedEffectLayer->GetImplementation())
        {
            p_serializableWidget->SetSerializable(impl);
            p_dirtParamsGo->SetEnabled(impl->GetEffectLayerType() ==
                                       EffectLayer::Type::DIRT);
            p_normalLinesParamsGo->SetEnabled(impl->GetEffectLayerType() ==
                                              EffectLayer::Type::NORMAL_LINES);
        }
    }

    p_serializableWidget->UpdateFromReference();

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
        CreateNewEffectLayer();
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
    Vector3 prevScale =
        GetView3DScene()->GetModelGameObject()->GetTransform()->GetLocalScale();
    GetView3DScene()->GetModelGameObject()->GetTransform()->SetLocalScale(
        GetView3DScene()->GetModelOriginalLocalScale());
    ModelIO::ExportModel(GetView3DScene()->GetModelGameObject(),
                         exportedModelPath);
    GetView3DScene()->GetModelGameObject()->GetTransform()->SetLocalScale(
        prevScale);
    GetView3DScene()->RestoreOriginalAlbedoTexturesToModel();
}

void ControlPanel::CreateNewEffectLayer()
{
    EffectLayer *newEL = GetView3DScene()->CreateNewEffectLayer();
    p_uiEffectLayers->CreateNewEffectLayerRow(newEL);
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

    m_params.normalLinesWidth = p_normalLinesWidthInput->GetValue();
    m_params.normalLinesHeight = p_normalLinesHeightInput->GetValue();

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

    p_normalLinesWidthInput->SetValue(GetParameters().normalLinesWidth);
    p_normalLinesHeightInput->SetValue(GetParameters().normalLinesHeight);

    EventListener<IEventsValueChanged>::SetReceiveEvents(true);
}

void ControlPanel::SetMaskUniforms(ShaderProgram *sp)
{
    sp->Bind();

    sp->SetBool("SeeMask", GetMaskBrushEnabled() && p_seeMaskButton->GetOn());
    sp->SetBool("MaskBrushEnabled", GetMaskBrushEnabled());
    sp->SetBool("MaskBrushDepthAware", p_maskBrushDepthAwareButton->GetOn());
    sp->SetBool("MaskBrushErasing", p_eraseMaskButton->GetOn());
    sp->SetVector2("MaskBrushCenter", Vector2(Input::GetMousePosition()));
    sp->SetFloat("MaskBrushHardness", GetMaskBrushHardness());
    sp->SetFloat("MaskBrushSize", GetMaskBrushSize());
    sp->SetFloat("MaskBrushStrength", p_maskBrushStrengthInput->GetValue());

    Array<EffectLayer *> effectLayers =
        GetView3DScene()->GetSelectedEffectLayers();
    if (effectLayers.Size() >= 1)
    {
        sp->SetTexture2D("MaskTexture", effectLayers.Front()->GetMaskTexture());
    }
}

void ControlPanel::ClearMask()
{
    Array<EffectLayer *> selectedEffectLayers =
        GetView3DScene()->GetSelectedEffectLayers();
    for (EffectLayer *selectedEffectLayer : selectedEffectLayers)
    {
        selectedEffectLayer->ClearMask();
    }
}

bool ControlPanel::GetMaskBrushEnabled() const
{
    return p_maskBrushEnabledButton->GetOn();
}

float ControlPanel::GetMaskBrushSize() const
{
    return p_maskBrushSizeInput->GetValue();
}

float ControlPanel::GetMaskBrushHardness() const
{
    return p_maskBrushHardnessInput->GetValue();
}

float ControlPanel::GetBaseRoughness() const
{
    return p_baseRoughnessInput->GetValue();
}

float ControlPanel::GetBaseMetalness() const
{
    return p_baseMetalnessInput->GetValue();
}

Vector2i ControlPanel::GetTextureSize() const
{
    return Vector2i(p_texturesSizeInput->GetSelectedValue(),
                    p_texturesSizeInput->GetSelectedValue());
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

void ControlPanel::OnValueChanged(EventEmitter<IEventsValueChanged> *ee)
{
    UpdateSelectedEffectLayerParameters();
}
