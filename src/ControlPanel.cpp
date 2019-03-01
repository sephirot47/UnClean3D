#include "ControlPanel.h"

#include "Bang/Dialog.h"
#include "Bang/Extensions.h"
#include "Bang/File.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Input.h"
#include "Bang/TextureFactory.h"
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
#include "BangEditor/UITabContainer.h"
#include "BangEditor/UITabHeader.h"

#include "BrushInspectorWidget.h"
#include "EffectLayer.h"
#include "EffectLayerCompositer.h"
#include "EffectLayerMask.h"
#include "EffectLayerMaskImplementation.h"
#include "MainScene.h"
#include "TextureContainer.h"
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

    GameObject *generalTab = GameObjectFactory::CreateUIGameObject();
    GameObject *sceneTab = GameObjectFactory::CreateUIGameObject();

    UILayoutElement *le = AddComponent<UILayoutElement>();
    le->SetMinWidth(600);

    UIImageRenderer *bg = AddComponent<UIImageRenderer>();
    bg->SetTint(Color::White().WithValue(0.8f));

    GameObjectFactory::AddOuterBorder(this, Vector2i(3), Color::Black());

    UITabContainer *tabContainer = new UITabContainer();
    tabContainer->AddTab("General", generalTab);
    tabContainer->AddTab("Scene", sceneTab);
    auto tabContLE = tabContainer->AddComponent<UILayoutElement>();
    tabContLE->SetMinWidth(600);
    tabContLE->SetLayoutPriority(99);
    tabContainer->SetParent(this);

    // General tab
    {
        UIVerticalLayout *vl = generalTab->AddComponent<UIVerticalLayout>();
        vl->SetPaddings(10);
        vl->SetSpacing(5);

        auto generalTabLE = generalTab->AddComponent<UILayoutElement>();
        generalTabLE->SetFlexibleSize(Vector2::One());

        // Open/Export buttons row
        {
            UILabel *fileSettingsLabel = GameObjectFactory::CreateUILabel();
            fileSettingsLabel->GetText()->SetContent("File settings");
            fileSettingsLabel->GetText()->SetTextSize(14);
            fileSettingsLabel->GetText()->SetHorizontalAlign(
                HorizontalAlignment::LEFT);
            CreateRow("", fileSettingsLabel->GetGameObject())
                ->SetParent(generalTab);

            GameObject *buttonsRow = GameObjectFactory::CreateUIGameObject();
            UIHorizontalLayout *hl =
                buttonsRow->AddComponent<UIHorizontalLayout>();
            hl->SetSpacing(10);
            UILayoutElement *rowLE =
                buttonsRow->AddComponent<UILayoutElement>();
            rowLE->SetMinHeight(20);
            buttonsRow->SetParent(generalTab);

            p_openModelButton = GameObjectFactory::CreateUIButton("Open");
            p_openModelButton->AddClickedCallback([this]() { OpenModel(); });
            p_openModelButton->GetGameObject()->SetParent(buttonsRow);

            p_exportModelButton = GameObjectFactory::CreateUIButton("Export");
            p_exportModelButton->AddClickedCallback(
                [this]() { ExportModel(); });
            p_exportModelButton->GetGameObject()->SetParent(buttonsRow);

            p_importEffectButton =
                GameObjectFactory::CreateUIButton("Import effect");
            p_importEffectButton->AddClickedCallback(
                [this]() { ImportEffect(); });
            p_importEffectButton->GetGameObject()->SetParent(buttonsRow);

            p_exportEffectButton =
                GameObjectFactory::CreateUIButton("Export effect");
            p_exportEffectButton->AddClickedCallback(
                [this]() { ExportEffect(); });
            p_exportEffectButton->GetGameObject()->SetParent(buttonsRow);

            GameObjectFactory::CreateUIHSeparator(LayoutSizeType::MIN, 15)
                ->SetParent(generalTab);
        }

        // View buttons row
        {
            UILabel *viewSettingsLabel = GameObjectFactory::CreateUILabel();
            viewSettingsLabel->GetText()->SetContent("View settings");
            viewSettingsLabel->GetText()->SetTextSize(14);
            viewSettingsLabel->GetText()->SetHorizontalAlign(
                HorizontalAlignment::LEFT);
            CreateRow("", viewSettingsLabel->GetGameObject())
                ->SetParent(generalTab);

            GameObject *sceneModeRow = GameObjectFactory::CreateUIGameObject();
            UIHorizontalLayout *hl =
                sceneModeRow->AddComponent<UIHorizontalLayout>();
            hl->SetSpacing(10);
            UILayoutElement *rowLE =
                sceneModeRow->AddComponent<UILayoutElement>();
            rowLE->SetMinHeight(20);
            sceneModeRow->SetParent(generalTab);

            UILabel *label = GameObjectFactory::CreateUILabel();
            label->GetText()->SetContent("Scene mode:");
            label->GetGameObject()->SetParent(sceneModeRow);

            p_sceneModeComboBox = GameObjectFactory::CreateUIComboBox();
            p_sceneModeComboBox->AddItem(
                "View 3D", SCAST<uint>(MainScene::SceneMode::VIEW3D));
            p_sceneModeComboBox->AddItem(
                "Textures", SCAST<uint>(MainScene::SceneMode::TEXTURES));

            // See effect or mask buttons
            {
                p_seeEffectButton =
                    GameObjectFactory::CreateUIToolButton("See effect");
                p_seeIsolatedMaskButton =
                    GameObjectFactory::CreateUIToolButton("See isolated mask");
                p_seeAccumulatedMaskButton =
                    GameObjectFactory::CreateUIToolButton(
                        "See accumulated mask");
                p_seeEffectButton->SetOn(true);
                p_seeIsolatedMaskButton->SetOn(false);
                p_seeAccumulatedMaskButton->SetOn(false);
                auto seeEffectOrMaskClick = [this](int pressedButton) {
                    p_seeEffectButton->SetOn(pressedButton == 0);
                    p_seeIsolatedMaskButton->SetOn(pressedButton == 1);
                    p_seeAccumulatedMaskButton->SetOn(pressedButton == 2);
                };
                p_seeEffectButton->AddClickedCallback(
                    [seeEffectOrMaskClick]() { seeEffectOrMaskClick(0); });
                p_seeIsolatedMaskButton->AddClickedCallback(
                    [seeEffectOrMaskClick]() { seeEffectOrMaskClick(1); });
                p_seeAccumulatedMaskButton->AddClickedCallback(
                    [seeEffectOrMaskClick]() { seeEffectOrMaskClick(2); });
                GameObject *seeEffectOrMaskButtonRow =
                    GameObjectFactory::CreateUIGameObject();
                auto hl = seeEffectOrMaskButtonRow
                              ->AddComponent<UIHorizontalLayout>();
                hl->SetSpacing(3);
                p_seeEffectButton->GetGameObject()->SetParent(
                    seeEffectOrMaskButtonRow);
                p_seeIsolatedMaskButton->GetGameObject()->SetParent(
                    seeEffectOrMaskButtonRow);
                p_seeAccumulatedMaskButton->GetGameObject()->SetParent(
                    seeEffectOrMaskButtonRow);
                CreateRow("See mode", seeEffectOrMaskButtonRow)
                    ->SetParent(generalTab);
            }

            p_sceneModeComboBox->GetGameObject()->SetParent(sceneModeRow);

            GameObjectFactory::CreateUIHSpacer(LayoutSizeType::FLEXIBLE,
                                               9999.0f)
                ->SetParent(sceneModeRow);
            GameObjectFactory::CreateUIHSeparator(LayoutSizeType::MIN, 30.0f)
                ->SetParent(generalTab);
        }

        // General settings
        {
            UILabel *generalSettingsLabel = GameObjectFactory::CreateUILabel();
            generalSettingsLabel->GetText()->SetContent("General settings");
            generalSettingsLabel->GetText()->SetTextSize(14);
            generalSettingsLabel->GetText()->SetHorizontalAlign(
                HorizontalAlignment::LEFT);
            CreateRow("", generalSettingsLabel->GetGameObject())
                ->SetParent(generalTab);

            p_baseRoughnessInput = GameObjectFactory::CreateUISlider(0, 1);
            p_baseRoughnessInput->SetValue(0.5f);
            p_baseRoughnessInput
                ->EventEmitter<IEventsValueChanged>::RegisterListener(this);
            CreateRow("Base roughness", p_baseRoughnessInput->GetGameObject())
                ->SetParent(generalTab);

            p_baseMetalnessInput = GameObjectFactory::CreateUISlider(0, 1);
            p_baseMetalnessInput->SetValue(0.5f);
            p_baseMetalnessInput
                ->EventEmitter<IEventsValueChanged>::RegisterListener(this);
            CreateRow("Base metalness", p_baseMetalnessInput->GetGameObject())
                ->SetParent(generalTab);

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
            CreateRow(
                "Textures size", p_texturesSizeInput->GetGameObject(), false)
                ->SetParent(generalTab);

            GameObjectFactory::CreateUIHSeparator(LayoutSizeType::MIN, 30.0f)
                ->SetParent(generalTab);
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

            p_effectLayerParamsGo->SetParent(generalTab);

            GameObjectFactory::CreateUIHSeparator(LayoutSizeType::MIN, 30.0f)
                ->SetParent(p_effectLayerParamsGo);
        }

        // Mask
        {
            p_maskParamsGo = GameObjectFactory::CreateUIGameObject();
            UIVerticalLayout *vl =
                p_maskParamsGo->AddComponent<UIVerticalLayout>();
            vl->SetSpacing(5);

            p_maskSubParamsGo = GameObjectFactory::CreateUIGameObject();
            UIVerticalLayout *subVL =
                p_maskSubParamsGo->AddComponent<UIVerticalLayout>();
            subVL->SetSpacing(5);

            p_maskLabel = GameObjectFactory::CreateUILabel();
            p_maskLabel->GetText()->SetContent("Mask");
            p_maskLabel->GetText()->SetTextSize(14);
            p_maskLabel->GetText()->SetHorizontalAlign(
                HorizontalAlignment::LEFT);
            CreateRow("", p_maskLabel->GetGameObject())
                ->SetParent(p_maskParamsGo);

            p_brushInspectorWidget = new BrushInspectorWidget();
            p_brushInspectorWidget->Init();
            p_brushInspectorWidget->SetParent(p_maskParamsGo);
            p_brushInspectorWidget->GetInspectorWidgetTitle()->SetEnabled(
                false);

            p_maskSerializableWidget = new SerializableInspectorWidget();
            p_maskSerializableWidget->Init();
            p_maskSerializableWidget->SetParent(p_maskParamsGo);
            p_maskSerializableWidget->GetInspectorWidgetTitle()->SetEnabled(
                false);

            p_maskSubParamsGo->SetParent(p_maskParamsGo);
            p_maskParamsGo->SetParent(generalTab);

            GameObjectFactory::CreateUIHSeparator(LayoutSizeType::MIN, 30.0f)
                ->SetParent(p_maskParamsGo);
        }

        // Effect layers
        {
            GameObjectFactory::CreateUIVSpacer(LayoutSizeType::FLEXIBLE, 99.9f)
                ->SetParent(generalTab);

            p_uiEffectLayers = new UIEffectLayers();
            UILayoutElement *le =
                p_uiEffectLayers->AddComponent<UILayoutElement>();
            le->SetMinHeight(60);
            le->SetPreferredHeight(450);
            le->SetFlexibleWidth(1.0f);
            p_uiEffectLayers->SetParent(generalTab);

            // GameObjectFactory::CreateUIHSeparator(LayoutSizeType::MIN, 30.0f)
            //     ->SetParent(generalTab);
        }
    }

    // Scene tab
    {
        UIVerticalLayout *vl = sceneTab->AddComponent<UIVerticalLayout>();
        vl->SetPaddings(10);
        vl->SetSpacing(5);

        auto sceneTabLE = sceneTab->AddComponent<UILayoutElement>();
        sceneTabLE->SetFlexibleSize(Vector2::One());

        UILabel *environmentLabel = GameObjectFactory::CreateUILabel();
        environmentLabel->GetText()->SetContent("Environment");
        environmentLabel->GetText()->SetTextSize(14);
        environmentLabel->GetText()->SetHorizontalAlign(
            HorizontalAlignment::LEFT);
        CreateRow("", environmentLabel->GetGameObject())->SetParent(sceneTab);

        p_rotateLightsInput = GameObjectFactory::CreateUICheckBox();
        p_rotateLightsInput->SetChecked(true);
        CreateRow("Rotate light", p_rotateLightsInput->GetGameObject())
            ->SetParent(sceneTab);

        GameObject *skyboxesRow = GameObjectFactory::CreateUIGameObject();
        {
            auto hl = skyboxesRow->AddComponent<UIHorizontalLayout>();
            hl->SetSpacing(20);

            struct SkyboxButton : public GameObject
            {
                View3DScene::Environment m_environment;
                TextureContainer *p_texCont = nullptr;
                SkyboxButton(View3DScene::Environment environment)
                {
                    m_environment = environment;
                    GameObjectFactory::CreateUIGameObjectInto(this);

                    AddComponent<UIVerticalLayout>();

                    p_texCont = new TextureContainer();
                    p_texCont->SetParent(this);
                    p_texCont->GetLabel()->GetGameObject()->SetEnabled(false);
                    p_texCont->SetCanBeFocused(true);
                    p_texCont->GetImageRenderer()->SetImageTexture(
                        View3DScene::GetInstance()->GetEnvironmentSnapshot(
                            environment));

                    Vector2i size(64);
                    p_texCont->GetImageLayoutElement()->SetMinSize(size);
                    auto le = AddComponent<UILayoutElement>();
                    le->SetMinSize(size);

                    auto focusable = p_texCont->GetFocusable();
                    focusable->AddEventCallback([this, environment](
                        UIFocusable *, const UIEvent &event) {
                        if (event.type == UIEvent::Type::MOUSE_CLICK_FULL)
                        {
                            View3DScene::GetInstance()->SetEnvironment(
                                environment);
                        }
                        return UIEventResult::IGNORE;
                    });
                }

                void Update() override
                {
                    GameObject::Update();
                    const bool selected =
                        (m_environment ==
                         View3DScene::GetInstance()->GetEnvironment());
                    p_texCont->SetSelected(selected);
                }
            };

            SkyboxButton *openSeaSkyBox =
                new SkyboxButton(View3DScene::Environment::OPEN_SEA);
            SkyboxButton *yokohamaSkyBox =
                new SkyboxButton(View3DScene::Environment::YOKOHAMA_NIGHT);
            SkyboxButton *parkSkyBox =
                new SkyboxButton(View3DScene::Environment::PARK);
            SkyboxButton *hotelSkyBox =
                new SkyboxButton(View3DScene::Environment::HOTEL);
            openSeaSkyBox->SetParent(skyboxesRow);
            yokohamaSkyBox->SetParent(skyboxesRow);
            parkSkyBox->SetParent(skyboxesRow);
            hotelSkyBox->SetParent(skyboxesRow);
        }
        skyboxesRow->SetParent(sceneTab);

        p_seeWithLightButton = GameObjectFactory::CreateUIToolButton("Light");
        p_seeWithLightButton->SetOn(true);
        CreateRow("", p_seeWithLightButton->GetGameObject())
            ->SetParent(sceneTab);
    }
}

ControlPanel::~ControlPanel()
{
}

void ControlPanel::Update()
{
    GameObject::Update();

    if (Input::GetKey(Key::LCTRL))
    {
        if (Input::GetKeyDown(Key::O))
        {
            OpenModel();
        }
        else if (Input::GetKeyDown(Key::E))
        {
            ExportModel();
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
                SerializableInspectorWidget *maskWidget =
                    p_maskSerializableWidget;
                p_maskSerializableWidget->SetEnabled(false);
                p_brushInspectorWidget->SetEnabled(false);
                if (impl->GetEffectLayerMaskType() ==
                    EffectLayerMask::Type::BRUSH)
                {
                    maskWidget = p_brushInspectorWidget;
                }
                maskWidget->SetEnabled(true);
                maskWidget->SetSerializable(impl);
                maskWidget->UpdateFromReference();

                enableMaskParams = true;
                enableEffectParams = false;
            }
        }
    }
    p_maskParamsGo->SetEnabled(enableMaskParams);
    p_effectLayerParamsGo->SetEnabled(enableEffectParams);

    if (!GetBrushTexture() &&
        p_brushInspectorWidget->GetTextureContainers().Size() >= 1)
    {
        SetBrushTexture(p_brushInspectorWidget->GetTextureContainers()
                            .Front()
                            ->GetImageRenderer()
                            ->GetImageTexture());
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
        p_uiEffectLayers->Clear();
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

void ControlPanel::ImportEffect()
{
    Path effectPath =
        Dialog::OpenFilePath("Import effect", {"fx"}, GetInitialDir());

    MetaNode meta;
    meta.Import(effectPath);

    const auto &effectLayerMetas = meta.GetChildren("EffectLayerChildren");
    for (const MetaNode &effectLayerMeta : effectLayerMetas)
    {
        UIEffectLayerRow *effectLayerRow = CreateNewEffectLayer();
        EffectLayer *effectLayer = effectLayerRow->GetEffectLayer();
        effectLayer->ImportMeta(effectLayerMeta);
    }

    UpdateFromEffectLayers();
}

void ControlPanel::ExportEffect()
{
    Path effectPath = Dialog::SaveFilePath(
        "Export effect", "fx", GetInitialDir(), "MyEffect");

    MetaNode wholeEffectMeta;
    wholeEffectMeta.SetName("Effect");
    auto allEffectLayers = GetView3DScene()->GetAllEffectLayers();
    for (auto it = allEffectLayers.RBegin(); it != allEffectLayers.REnd(); ++it)
    {
        EffectLayer *effectLayer = *it;
        MetaNode effectMeta;
        effectLayer->ExportMeta(&effectMeta);
        wholeEffectMeta.AddChild(effectMeta, "EffectLayerChildren");
    }

    File::Write(effectPath, wholeEffectMeta.ToString());
}

void ControlPanel::UpdateFromEffectLayers()
{
    p_uiEffectLayers->UpdateFromEffectLayers();
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
    p_uiEffectLayers->RemoveEffectLayerRow(effectLayerIdx, false);
}

void ControlPanel::SetControlPanelUniforms(ShaderProgram *sp)
{
    sp->Bind();

    sp->SetBool("WithLight", p_seeWithLightButton->GetOn());

    bool seeMask = (GetSeeMode() == SeeMode::ACCUM_MASK);
    seeMask |= (GetSeeMode() == SeeMode::ISOLATED_MASK &&
                GetSelectedEffectLayerMask());

    Texture2D *maskTexToSee = TextureFactory::GetWhiteTexture();
    if (seeMask)
    {
        if (GetSeeMode() == SeeMode::ACCUM_MASK)
        {
            if (GetSelectedEffectLayer())
            {
                maskTexToSee = GetSelectedEffectLayer()->GetMergedMaskTexture();
            }
            else
            {
                seeMask = false;
            }
        }
        else
        {
            if (GetSelectedEffectLayerMask())
            {
                maskTexToSee = GetSelectedEffectLayerMask()
                                   ->GetImplementation()
                                   ->GetMaskTextureToSee();
            }
            else
            {
                seeMask = false;
            }
        }
    }

    sp->SetBool("SeeMask", seeMask);
    sp->SetTexture2D("MaskBrushTexture",
                     ControlPanel::GetInstance()->GetBrushTexture());
    sp->SetTexture2D("MaskTextureToSee", maskTexToSee);
}

void ControlPanel::SetBrushTexture(Texture2D *brushTexture)
{
    p_brushTexture.Set(brushTexture);
}

bool ControlPanel::GetMaskBrushEnabled() const
{
    return (GetSelectedEffectLayerMask() &&
            (GetSelectedEffectLayerMask()->GetType() ==
             EffectLayerMask::Type::BRUSH));
}

void ControlPanel::ReloadShaders()
{
}

float ControlPanel::GetBaseRoughness() const
{
    return p_baseRoughnessInput->GetValue();
}

float ControlPanel::GetBaseMetalness() const
{
    return p_baseMetalnessInput->GetValue();
}

bool ControlPanel::GetRotateLights() const
{
    return p_rotateLightsInput->IsChecked();
}

Vector2i ControlPanel::GetTextureSize() const
{
    return Vector2i(p_texturesSizeInput->GetSelectedValue(),
                    p_texturesSizeInput->GetSelectedValue());
}

Texture2D *ControlPanel::GetBrushTexture() const
{
    return p_brushTexture.Get();
}

ControlPanel::SeeMode ControlPanel::GetSeeMode() const
{
    if (p_seeEffectButton->GetOn())
    {
        return SeeMode::EFFECT;
    }
    else if (p_seeIsolatedMaskButton->GetOn())
    {
        return SeeMode::ISOLATED_MASK;
    }
    return SeeMode::ACCUM_MASK;
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

ControlPanel *ControlPanel::GetInstance()
{
    return MainScene::GetInstance()->GetControlPanel();
}

Path ControlPanel::GetInitialDir() const
{
    return EditorPaths::GetExecutableDir().GetDirectory().Append("Assets");
}

Path ControlPanel::GetOpenModelPath() const
{
    return m_openModelPath;
}

View3DScene *ControlPanel::GetView3DScene() const
{
    return MainScene::GetInstance()->GetView3DScene();
}

GameObject *ControlPanel::CreateRow(const String &labelStr,
                                    GameObject *go,
                                    bool stretch)
{
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
}

void ControlPanel::OnValueChanged(EventEmitter<IEventsValueChanged> *)
{
    GetView3DScene()->InvalidateAll();
}
