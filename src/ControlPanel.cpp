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
    vl->SetSpacing(5);

    GameObjectFactory::AddOuterBorder(this, Vector2i(3), Color::Black());

    UILayoutElement *le = AddComponent<UILayoutElement>();
    le->SetMinWidth(600);

    UIImageRenderer *bg = AddComponent<UIImageRenderer>();
    bg->SetTint(Color::White().WithValue(0.8f));

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

    // Effect Layer params
    {
        p_effectLayerParamsGo = GameObjectFactory::CreateUIGameObject();
        UIVerticalLayout *vl =
            p_effectLayerParamsGo->AddComponent<UIVerticalLayout>();
        vl->SetSpacing(5);

        p_effectLayerParamsTitle = GameObjectFactory::CreateUILabel();
        p_effectLayerParamsTitle->GetText()->SetContent("Title");
        p_effectLayerParamsTitle->GetText()->SetTextSize(14);
        p_effectLayerParamsTitle->GetText()->SetHorizontalAlign(
            HorizontalAlignment::LEFT);
        CreateRow("", p_effectLayerParamsTitle->GetGameObject())
            ->SetParent(p_effectLayerParamsGo);

        p_serializableWidget = new SerializableInspectorWidget();
        p_serializableWidget->Init();

        p_serializableWidget->SetParent(p_effectLayerParamsGo);
        p_effectLayerParamsGo->SetParent(this);

        p_serializableWidget->GetInspectorWidgetTitle()->SetEnabled(false);

        GameObjectFactory::CreateUIHSeparator(LayoutSizeType::MIN, 30.0f)
            ->SetParent(this);
    }

    // Mask
    {
        p_maskParamsGo = GameObjectFactory::CreateUIGameObject();
        UIVerticalLayout *vl = p_maskParamsGo->AddComponent<UIVerticalLayout>();
        vl->SetSpacing(5);

        p_maskSubParamsGo = GameObjectFactory::CreateUIGameObject();
        UIVerticalLayout *subVL =
            p_maskSubParamsGo->AddComponent<UIVerticalLayout>();
        subVL->SetSpacing(5);

        UILabel *maskLabel = GameObjectFactory::CreateUILabel();
        maskLabel->GetText()->SetContent("Mask");
        maskLabel->GetText()->SetTextSize(14);
        maskLabel->GetText()->SetHorizontalAlign(HorizontalAlignment::LEFT);
        CreateRow("", maskLabel->GetGameObject())->SetParent(p_maskParamsGo);

        p_maskBrushDrawButton =
            GameObjectFactory::CreateUIToolButton("Draw Mask (M)");
        p_maskBrushDrawButton->SetOn(false);
        p_maskBrushDrawButton->GetGameObject()->SetParent(p_maskParamsGo);

        p_maskBrushSizeInput = GameObjectFactory::CreateUISlider(1, 500);
        p_maskBrushSizeInput->SetValue(50.0f);
        p_maskBrushSizeInputRow =
            CreateRow("Brush size", p_maskBrushSizeInput->GetGameObject());
        p_maskBrushSizeInputRow->SetParent(p_maskSubParamsGo);

        p_maskBrushHardnessInput = GameObjectFactory::CreateUISlider(0, 1);
        p_maskBrushHardnessInput->SetValue(0.1f);
        p_maskBrushHardnessInputRow = CreateRow(
            "Brush hardness", p_maskBrushHardnessInput->GetGameObject());
        p_maskBrushHardnessInputRow->SetParent(p_maskSubParamsGo);

        p_maskBrushStrengthInput = GameObjectFactory::CreateUISlider(0, 1);
        p_maskBrushStrengthInput->SetValue(1.0f);
        p_maskBrushStrengthInputRow = CreateRow(
            "Brush strength", p_maskBrushStrengthInput->GetGameObject());
        p_maskBrushStrengthInputRow->SetParent(p_maskSubParamsGo);

        p_seeMaskButton =
            GameObjectFactory::CreateUIToolButton("See Mask (Shift + M)");
        p_seeMaskButton->SetOn(false);
        p_seeMaskButton->GetGameObject()->SetParent(p_maskSubParamsGo);

        p_maskBrushDepthAwareButton =
            GameObjectFactory::CreateUIToolButton("Depth aware mask brush");
        p_maskBrushDepthAwareButton->SetOn(true);
        p_maskBrushDepthAwareButton->GetGameObject()->SetParent(
            p_maskSubParamsGo);

        p_eraseMaskButton =
            GameObjectFactory::CreateUIToolButton("Erase Mask (Ctrl)");
        p_eraseMaskButton->SetOn(false);
        p_eraseMaskButton->GetGameObject()->SetParent(p_maskSubParamsGo);

        p_clearMaskButton = GameObjectFactory::CreateUIButton("Clear Mask (C)");
        p_clearMaskButton->AddClickedCallback([this]() { ClearMask(); });
        p_clearMaskButton->GetGameObject()->SetParent(p_maskSubParamsGo);

        p_fillMaskButton = GameObjectFactory::CreateUIButton("Fill Mask");
        p_fillMaskButton->AddClickedCallback([this]() { FillMask(); });
        p_fillMaskButton->GetGameObject()->SetParent(p_maskSubParamsGo);

        p_maskSubParamsGo->SetParent(p_maskParamsGo);
        p_maskParamsGo->SetParent(this);

        GameObjectFactory::CreateUIHSeparator(LayoutSizeType::MIN, 30.0f)
            ->SetParent(p_maskParamsGo);
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

    bool enableParams = false;
    Array<EffectLayer *> selectedEffectLayers =
        GetView3DScene()->GetSelectedEffectLayers();
    if (selectedEffectLayers.Size() >= 1)
    {
        EffectLayer *selectedEffectLayer = selectedEffectLayers.Front();

        if (EffectLayerImplementation *impl =
                selectedEffectLayer->GetImplementation())
        {
            p_effectLayerParamsTitle->GetText()->SetContent(
                impl->GetTypeName());
            p_serializableWidget->SetSerializable(impl);
            p_serializableWidget->UpdateFromReference();
            enableParams = true;
        }
    }

    p_effectLayerParamsGo->SetEnabled(enableParams);
    p_maskParamsGo->SetEnabled(enableParams);
    p_maskSubParamsGo->SetEnabled(GetMaskBrushEnabled());

    if (enableParams)
    {
        p_eraseMaskButton->SetOn(GetMaskBrushEnabled() &&
                                 Input::GetKey(Key::LCTRL));
        if (Input::GetKeyDown(Key::M) && !Input::GetKey(Key::LSHIFT))
        {
            p_maskBrushDrawButton->SetOn(!p_maskBrushDrawButton->GetOn());
        }
        else if (Input::GetKeyDown(Key::C))
        {
            ClearMask();
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

void ControlPanel::FillMask()
{
    Array<EffectLayer *> selectedEffectLayers =
        GetView3DScene()->GetSelectedEffectLayers();
    for (EffectLayer *selectedEffectLayer : selectedEffectLayers)
    {
        selectedEffectLayer->FillMask();
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
    return p_maskBrushDrawButton->GetOn();
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
    GetView3DScene()->InvalidateTextures();
}
