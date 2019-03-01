#pragma once

#include "Bang/Bang.h"
#include "Bang/GameObject.h"

class MainScene;

namespace Bang
{
class Texture2D;
class UIFocusable;
class UIImageRenderer;
}

class SceneImage : public GameObject
{
public:
    SceneImage();
    virtual ~SceneImage() override;

    void Render(RenderPass renderPass, bool renderChildren) override;

    void SetImageTexture(Texture2D *texture);

    UIFocusable *GetFocusable() const;

private:
    UIFocusable *p_focusable = nullptr;
    UIImageRenderer *p_sceneImgRend = nullptr;
};
