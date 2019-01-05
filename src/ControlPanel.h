#pragma once

#include "Bang/Bang.h"
#include "Bang/GameObject.h"
#include "Bang/UIButton.h"
#include "Bang/UIComboBox.h"
#include "Bang/UIToolButton.h"

#include "MainScene.h"

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

    void SetSceneModeOnComboBox(MainScene::SceneMode sceneMode);

private:
    UIButton *p_openModelButton = nullptr;
    UIButton *p_exportModelButton = nullptr;
    UIComboBox *p_sceneModeComboBox = nullptr;

    Path m_openModelPath = Path::Empty();

    Path GetInitialDir() const;
    Path GetOpenModelPath() const;
};
