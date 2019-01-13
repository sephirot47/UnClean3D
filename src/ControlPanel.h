#pragma once

#include "Bang/Bang.h"
#include "Bang/EventListener.h"
#include "Bang/GameObject.h"
#include "Bang/IEventsValueChanged.h"

#include "EffectLayerParameters.h"
#include "MainScene.h"

namespace Bang
{
class UIButton;
class UISlider;
class UIComboBox;
class UIToolButton;
class UIInputNumber;
}

namespace BangEditor
{
class UIInputColor;
}

class UIEffectLayers;
class View3DScene;
using namespace BangEditor;

class ControlPanel : public GameObject,
                     public EventListener<IEventsValueChanged>
{
public:
    ControlPanel();
    virtual ~ControlPanel() override;

    void Update() override;

    void OpenModel();
    void OpenModel(const Path &modelPath);
    void ExportModel();

    void CreateNewEffectLayer();
    void RemoveEffectLayer(uint effectLayerIdx);
    void UpdateSelectedEffectLayerParameters();
    void UpdateInputsAndParametersFromSelectedEffectLayer();

    uint GetSelectedUIEffectLayerIndex() const;
    bool IsVisibleUIEffectLayer(uint effectLayerIdx) const;

    const EffectLayerParameters &GetParameters() const;
    void SetSceneModeOnComboBox(MainScene::SceneMode sceneMode);

private:
    Path m_openModelPath = Path::Empty();
    EffectLayerParameters m_params;

    // File
    UIButton *p_openModelButton = nullptr;
    UIButton *p_exportModelButton = nullptr;

    // General
    UIComboBox *p_sceneModeComboBox = nullptr;

    // Dirt
    GameObject *p_dirtParamsGo = nullptr;
    UIInputNumber *p_dirtSeedInput = nullptr;
    UIInputColor *p_dirtColor0Input = nullptr;
    UIInputColor *p_dirtColor1Input = nullptr;
    UISlider *p_dirtFrequencyInput = nullptr;
    UISlider *p_dirtAmplitudeInput = nullptr;
    UISlider *p_dirtFrequencyMultiplyInput = nullptr;
    UISlider *p_dirtAmplitudeMultiplyInput = nullptr;

    // Effect layers
    UIEffectLayers *p_uiEffectLayers = nullptr;

    Path GetInitialDir() const;
    Path GetOpenModelPath() const;
    View3DScene *GetView3DScene() const;

    // IEventsValueChanged
    virtual void OnValueChanged(EventEmitter<IEventsValueChanged> *ee) override;
};
