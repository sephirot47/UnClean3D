#pragma once

#include "Bang/Bang.h"
#include "Bang/GameObject.h"
#include "Bang/UIImageRenderer.h"

class MainScene;

class SceneImage : public GameObject
{
public:
    SceneImage();
    virtual ~SceneImage() override;

    void Render(RenderPass renderPass, bool renderChildren) override;

    void SetImageTexture(Texture2D *texture);

private:
    UIImageRenderer *p_sceneImgRend = nullptr;
};
