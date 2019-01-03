#include "SceneImage.h"

#include "Bang/GBuffer.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/RectTransform.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UIVerticalLayout.h"
#include "EditScene.h"

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
    if (renderPass == RenderPass::CANVAS)
    {
        p_editScene->Update();

        Vector2i renderSize(p_sceneImgRend->GetGameObject()
                                ->GetRectTransform()
                                ->GetViewportAARect()
                                .GetSize());
        p_editScene->RenderScene(renderSize);

        p_sceneImgRend->SetImageTexture(
            p_editScene->GetCamera()->GetGBuffer()->GetDrawColorTexture());
    }

    GameObject::Render(renderPass, renderChildren);
}

void SceneImage::SetEditScene(EditScene *editScene)
{
    p_editScene = editScene;
}
