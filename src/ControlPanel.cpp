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
#include "EffectLayerMask.h"
#include "EffectLayerMaskImplementation.h"
#include "MainScene.h"
#include "UIEffectLayerMaskRow.h"
#include "UIEffectLayerParameters.h"
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
        p_baseRoughnessInput->SetValue(0.5f);
        p_baseRoughnessInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);
        CreateRow("Base roughness", p_baseRoughnessInput->GetGameObject())
            ->SetParent(this);

        p_baseMetalnessInput = GameObjectFactory::CreateUISlider(0, 1);
        p_baseMetalnessInput->SetValue(0.5f);
        p_baseMetalnessInput
            ->EventEmitter<IEventsValueChanged>::RegisterListener(this);
        CreateRow("Base metalness", p_baseMetalnessInput->GetGameObject())
            ->SetParent(this);

        p_seeWithLightButton = GameObjectFactory::CreateUIToolButton("Light");
        p_seeWithLightButton->SetOn(true);
        CreateRow("", p_seeWithLightButton->GetGameObject())->SetParent(this);

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

        p_effectParametersWidget = new UIEffectLayerParameters();
        p_effectParametersWidget->SetParent(p_effectLayerParamsGo);

        p_effectLayerParamsGo->SetParent(this);

        GameObjectFactory::CreateUIHSeparator(LayoutSizeType::MIN, 30.0f)
            ->SetParent(p_effectLayerParamsGo);
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

        p_maskLabel = GameObjectFactory::CreateUILabel();
        p_maskLabel->GetText()->SetContent("Mask");
        p_maskLabel->GetText()->SetTextSize(14);
        p_maskLabel->GetText()->SetHorizontalAlign(HorizontalAlignment::LEFT);
        CreateRow("", p_maskLabel->GetGameObject())->SetParent(p_maskParamsGo);

        p_maskSerializableWidget = new SerializableInspectorWidget();
        p_maskSerializableWidget->Init();
        p_maskSerializableWidget->SetParent(p_maskParamsGo);
        p_maskSerializableWidget->GetInspectorWidgetTitle()->SetEnabled(false);

        p_seeMaskButton =
            GameObjectFactory::CreateUIToolButton("See Mask (Shift + M)");
        p_seeMaskButton->SetOn(false);
        p_seeMaskButton->GetGameObject()->SetParent(p_maskSubParamsGo);

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

    if (Input::GetKey(Key::LSHIFT))
    {
        if (Input::GetKeyDown(Key::M))
        {
            p_seeMaskButton->SetOn(!p_seeMaskButton->GetOn());
        }
    }

    bool enableEffectParams = false;
    bool enableMaskParams = false;
    if (EffectLayer *selectedEffectLayer = GetSelectedEffectLayer())
    {
        p_effectLayerParamsTitle->GetText()->SetContent("Effect Layer");
        p_effectParametersWidget->SetEffectLayer(selectedEffectLayer);
        enableEffectParams = true;

        if (EffectLayerMask *selectedEffectLayerMask =
                GetSelectedEffectLayerMask())
        {
            if (EffectLayerMaskImplementation *impl =
                    selectedEffectLayerMask->GetImplementation())
            {
                p_maskLabel->GetText()->SetContent("Mask " +
                                                   impl->GetTypeName());
                p_maskSerializableWidget->SetSerializable(impl);
                p_maskSerializableWidget->UpdateFromReference();
                enableMaskParams = true;
                enableEffectParams = false;
            }
        }
    }
    p_maskParamsGo->SetEnabled(enableMaskParams);
    p_effectLayerParamsGo->SetEnabled(enableEffectParams);

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

UIEffectLayerRow *ControlPanel::CreateNewEffectLayer()
{
    EffectLayer *newEL = GetView3DScene()->CreateNewEffectLayer();
    UIEffectLayerRow *newELRow =
        p_uiEffectLayers->CreateNewEffectLayerRow(newEL);
    return newELRow;
}

void ControlPanel::RemoveEffectLayer(uint effectLayerIdx)
{
    GetView3DScene()->RemoveEffectLayer(effectLayerIdx);
    p_uiEffectLayers->RemoveEffectLayer(effectLayerIdx);
}

void ControlPanel::SetControlPanelUniforms(ShaderProgram *sp)
{
    sp->Bind();
    sp->SetBool("WithLight", p_seeWithLightButton->GetOn());
    sp->SetBool("SeeMask",
                GetSelectedEffectLayerMask() && p_seeMaskButton->GetOn());
}

bool ControlPanel::GetMaskBrushEnabled() const
{
    return (GetSelectedEffectLayerMask() &&
            (GetSelectedEffectLayerMask()->GetType() ==
             EffectLayerMask::Type::BRUSH));
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

EffectLayer *ControlPanel::GetSelectedEffectLayer() const
{
    if (GetSelectedEffectLayerRow())
    {
        return GetSelectedEffectLayerRow()->GetEffectLayer();
    }
    return nullptr;
}

EffectLayerMask *ControlPanel::GetSelectedEffectLayerMask() const
{
    if (GetSelectedEffectLayerMaskRow())
    {
        return GetSelectedEffectLayerMaskRow()->GetEffectLayerMask();
    }
    return nullptr;
}

UIEffectLayerRow *ControlPanel::GetSelectedEffectLayerRow() const
{
    return p_uiEffectLayers->GetSelectedEffectLayerRow();
}

UIEffectLayerMaskRow *ControlPanel::GetSelectedEffectLayerMaskRow() const
{
    if (UIEffectLayerRow *selectedLayerRow = GetSelectedEffectLayerRow())
    {
        return SCAST<UIEffectLayerMaskRow *>(
            selectedLayerRow->GetMaskRowsList()->GetSelectedItem());
    }
    return nullptr;
}

uint ControlPanel::GetSelectedUIEffectLayerIndex() const
{
    return p_uiEffectLayers->GetSelectedEffectLayerRowIndex();
}

uint ControlPanel::GetSelectedUIEffectLayerMaskIndex() const
{
    if (UIEffectLayerRow *selectedEffectLayerRow =
            p_uiEffectLayers->GetSelectedEffectLayerRow())
    {
        return selectedEffectLayerRow->GetMaskRowsList()->GetSelectedIndex();
    }
    return -1u;
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
    GetView3DScene()->InvalidateAll();
}
