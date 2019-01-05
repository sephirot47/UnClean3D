#include "TexturesScene.h"

#include "Bang/Array.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Material.h"
#include "Bang/MeshRenderer.h"
#include "Bang/Texture2D.h"
#include "Bang/UIGridLayout.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UIVerticalLayout.h"

#include "MainScene.h"
#include "View3DScene.h"

using namespace Bang;

TexturesScene::TexturesScene()
{
    GameObjectFactory::CreateUISceneInto(this);

    UIVerticalLayout *vl = AddComponent<UIVerticalLayout>();
    vl->SetSpacing(5);

    GameObject *gridGo = GameObjectFactory::CreateUIGameObject();
    UILayoutElement *gridLE = gridGo->AddComponent<UILayoutElement>();
    gridLE->SetFlexibleSize(Vector2(1.0f));
    UIGridLayout *gl = gridGo->AddComponent<UIGridLayout>();
    gl->SetCellSize(Vector2i(512));
    gl->SetSpacing(10);
    gl->SetPaddings(10);

    p_albedoImgRend = GameObjectFactory::CreateUIImage();
    UILayoutElement *imgLE =
        p_albedoImgRend->GetGameObject()->AddComponent<UILayoutElement>();
    imgLE->SetFlexibleSize(Vector2(1.0f));
    p_albedoImgRend->GetGameObject()->SetParent(gridGo);

    UIImageRenderer *img2 = GameObjectFactory::CreateUIImage();
    UILayoutElement *img2LE =
        img2->GetGameObject()->AddComponent<UILayoutElement>();
    img2LE->SetFlexibleSize(Vector2(1.0f));
    img2->SetTint(Color::Green());
    img2->GetGameObject()->SetParent(gridGo);

    gridGo->SetParent(this);

    UIImageRenderer *bg = AddComponent<UIImageRenderer>();
    bg->SetTint(Color::Black());
}

TexturesScene::~TexturesScene()
{
}

void TexturesScene::Update()
{
    GameObject::Update();

    View3DScene *viewScene = MainScene::GetInstance()->GetView3DScene();
    if (GameObject *modelGo = viewScene->GetModelGameObject())
    {
        Array<MeshRenderer *> mrs =
            modelGo->GetComponentsInDescendantsAndThis<MeshRenderer>();
        for (MeshRenderer *mr : mrs)
        {
            if (Material *mat = mr->GetActiveMaterial())
            {
                if (Texture2D *albedoTex = mat->GetAlbedoTexture())
                {
                    p_albedoImgRend->SetImageTexture(albedoTex);
                }
            }
        }
    }
}

void TexturesScene::OnModelChanged(Model *)
{
}
