#include "SceneImage.h"

#include "Bang/GBuffer.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/RectTransform.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UIVerticalLayout.h"

#include "MainScene.h"
#include "View3DScene.h"

SceneImage::SceneImage()
{
    GameObjectFactory::CreateUIGameObjectInto(this);

    UILayoutElement *le = AddComponent<UILayoutElement>();
    le->SetFlexibleWidth(1.0f);

    p_sceneImgRend = AddComponent<UIImageRenderer>();
    p_sceneImgRend->SetTint(Color::White());
    p_sceneImgRend->SetMode(UIImageRenderer::Mode::TEXTURE);
}

SceneImage::~SceneImage()
{
}

void SceneImage::Render(RenderPass renderPass, bool renderChildren)
{
    GameObject::Render(renderPass, renderChildren);
}

void SceneImage::SetImageTexture(Texture2D *texture)
{
    p_sceneImgRend->SetImageTexture(texture);
}
