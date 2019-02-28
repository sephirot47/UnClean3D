#pragma once

#include "Bang/Bang.h"
#include "Bang/EventListener.h"
#include "Bang/GameObject.h"
#include "Bang/IEventsValueChanged.h"

#include "MainScene.h"

namespace Bang
{
class UIButton;
class UILabel;
class UISlider;
class UIComboBox;
class UIToolButton;
class UIInputNumber;
}

namespace BangEditor
{
class UIInputColor;
class SerializableInspectorWidget;
}

class EffectLayer;
class BrushInspectorWidget;
class EffectLayerMask;
class UIEffectLayerParameters;
class UIEffectLayers;
class UIEffectLayerRow;
class UIEffectLayerMaskRow;
class View3DScene;
using namespace BangEditor;

class ControlPanel : public GameObject,
                     public EventListener<IEventsValueChanged>
{
public:
    enum class SeeMode
    {
        EFFECT,
        ISOLATED_MASK,
        ACCUM_MASK
    };

    ControlPanel();
    virtual ~ControlPanel() override;

    void Update() override;

    void OpenModel();
    void OpenModel(const Path &modelPath);
    void ExportModel();
    void ImportEffect();
    void ExportEffect();

    void UpdateFromEffectLayers();
    UIEffectLayerRow *CreateNewEffectLayer();
    void RemoveEffectLayer(uint effectLayerIdx);
    void SetControlPanelUniforms(ShaderProgram *sp);

    void FillSelectedMask();
    void ClearSelectedMask();
    void SetBrushTexture(Texture2D *brushTexture);
    bool GetMaskBrushEnabled() const;

    void ReloadShaders();
    float GetBaseRoughness() const;
    float GetBaseMetalness() const;
    Vector2i GetTextureSize() const;
    Texture2D *GetBrushTexture() const;
    ControlPanel::SeeMode GetSeeMode() const;
    EffectLayer *GetSelectedEffectLayer() const;
    EffectLayerMask *GetSelectedEffectLayerMask() const;
    UIEffectLayerRow *GetSelectedEffectLayerRow() const;
    UIEffectLayerMaskRow *GetSelectedEffectLayerMaskRow() const;
    uint GetSelectedUIEffectLayerIndex() const;
    uint GetSelectedUIEffectLayerMaskIndex() const;

    void SetSceneModeOnComboBox(MainScene::SceneMode sceneMode);

    static ControlPanel *GetInstance();

private:
    Path m_openModelPath = Path::Empty();

    // File
    UIButton *p_openModelButton = nullptr;
    UIButton *p_exportModelButton = nullptr;
    UIButton *p_importEffectButton = nullptr;
    UIButton *p_exportEffectButton = nullptr;

    // View
    UIToolButton *p_seeEffectButton = nullptr;
    UIToolButton *p_seeIsolatedMaskButton = nullptr;
    UIToolButton *p_seeAccumulatedMaskButton = nullptr;
    UIToolButton *p_seeWithLightButton = nullptr;

    // General
    UIComboBox *p_sceneModeComboBox = nullptr;
    UISlider *p_baseRoughnessInput = nullptr;
    UISlider *p_baseMetalnessInput = nullptr;
    UIComboBox *p_texturesSizeInput = nullptr;

    // Mask
    GameObject *p_maskParamsGo = nullptr;
    GameObject *p_maskSubParamsGo = nullptr;
    UILabel *p_maskLabel = nullptr;
    AH<Texture2D> p_brushTexture;

    // Effect layers
    UIEffectLayers *p_uiEffectLayers = nullptr;

    // Effect layer params
    GameObject *p_effectLayerParamsGo = nullptr;
    UILabel *p_effectLayerParamsTitle = nullptr;

    // SerializableWidget
    UIEffectLayerParameters *p_effectParametersWidget = nullptr;
    SerializableInspectorWidget *p_maskSerializableWidget = nullptr;
    BrushInspectorWidget *p_brushInspectorWidget = nullptr;

    Path GetInitialDir() const;
    Path GetOpenModelPath() const;
    View3DScene *GetView3DScene() const;

    GameObject *CreateRow(const String &labelStr = "",
                          GameObject *go = nullptr,
                          bool stretch = true);

    // IEventsValueChanged
    virtual void OnValueChanged(EventEmitter<IEventsValueChanged> *ee) override;
};
