#pragma once

#include "Bang/Bang.h"
#include "Bang/GameObject.h"

class EditScene;
class ControlPanel : public GameObject
{
public:
    ControlPanel();
    virtual ~ControlPanel() override;

    void OpenModel();
    void ExportModel();

    void SetEditScene(EditScene *editScene);

private:
    EditScene *p_editScene = nullptr;
};
