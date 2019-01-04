#pragma once

#include "Bang/Bang.h"
#include "Bang/GameObject.h"
#include "Bang/UIButton.h"

class EditScene;

class ControlPanel : public GameObject
{
public:
    ControlPanel();
    virtual ~ControlPanel() override;

    void Update() override;

    void OpenModel();
    void OpenModel(const Path &modelPath);
    void ExportModel();

    void SetEditScene(EditScene *editScene);

private:
    UIButton *p_openModelButton = nullptr;
    UIButton *p_exportModelButton = nullptr;

    Path m_openModelPath = Path::Empty();
    EditScene *p_editScene = nullptr;

    Path GetInitialDir() const;
    Path GetOpenModelPath() const;
};
