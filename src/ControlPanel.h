#pragma once

#include "Bang/Bang.h"
#include "Bang/EventListener.h"
#include "Bang/GameObject.h"
#include "Bang/IEventsValueChanged.h"

#include "MainScene.h"

namespace Bang
{
class UIButton;
class UISlider;
class UIComboBox;
class UIToolButton;
class UIInputNumber;
}
class View3DScene;

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

    float GetDirtOctaves() const;
    float GetDirtFrequency() const;
    float GetDirtAmplitude() const;
    float GetDirtFrequencyMultiply() const;
    float GetDirtAmplitudeMultiply() const;

    void SetSceneModeOnComboBox(MainScene::SceneMode sceneMode);

private:
    Path m_openModelPath = Path::Empty();

    // File
    UIButton *p_openModelButton = nullptr;
    UIButton *p_exportModelButton = nullptr;

    // General
    UIComboBox *p_sceneModeComboBox = nullptr;

    // Dirt
    UISlider *p_dirtFactorInput = nullptr;
    UISlider *p_dirtOctavesInput = nullptr;
    UISlider *p_dirtFrequencyInput = nullptr;
    UISlider *p_dirtAmplitudeInput = nullptr;
    UISlider *p_dirtFrequencyMultiplyInput = nullptr;
    UISlider *p_dirtAmplitudeMultiplyInput = nullptr;

    Path GetInitialDir() const;
    Path GetOpenModelPath() const;

    // IEventsValueChanged
    virtual void OnValueChanged(EventEmitter<IEventsValueChanged> *ee) override;
};
