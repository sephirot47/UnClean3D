#pragma once

#include "Bang/Bang.h"
#include "Bang/GameObject.h"

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

class ControlPanel : public GameObject
{
public:
    ControlPanel();
    virtual ~ControlPanel() override;

    void Update() override;

    void OpenModel();
    void OpenModel(const Path &modelPath);
    void ExportModel();

    float GetDirtFactor() const;

    void SetSceneModeOnComboBox(MainScene::SceneMode sceneMode);

private:
    UIButton *p_openModelButton = nullptr;
    UIButton *p_exportModelButton = nullptr;
    UIComboBox *p_sceneModeComboBox = nullptr;

    UISlider *p_dirtFactorInput = nullptr;
    Path m_openModelPath = Path::Empty();

    Path GetInitialDir() const;
    Path GetOpenModelPath() const;
};
