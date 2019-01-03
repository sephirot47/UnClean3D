#pragma once

#include "Bang/Bang.h"
#include "Bang/GameObject.h"
#include "Bang/UIImageRenderer.h"

class EditScene;

class SceneImage : public GameObject
{
public:
    SceneImage();
    virtual ~SceneImage() override;

    void Render(RenderPass renderPass, bool renderChildren) override;

    void SetEditScene(EditScene *editScene);

private:
    EditScene *p_editScene = nullptr;
    UIImageRenderer *p_sceneImgRend = nullptr;
};
