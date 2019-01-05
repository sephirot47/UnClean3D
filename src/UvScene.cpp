#include "UvScene.h"

#include "Bang/Camera.h"
#include "Bang/DirectionalLight.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Model.h"
#include "Bang/Transform.h"

using namespace Bang;

UvScene::UvScene()
{
    AddComponent<Transform>();

    GameObject *camGo = GameObjectFactory::CreateGameObject();
    p_cam = GameObjectFactory::CreateDefaultCameraInto(camGo);
    p_cam->SetZFar(100.0f);
    camGo->SetParent(this);
    SetCamera(p_cam);

    GameObject *dlGo = GameObjectFactory::CreateGameObject();
    DirectionalLight *dl = dlGo->AddComponent<DirectionalLight>();
    dl->SetShadowStrength(1.0f);
    dl->SetShadowDistance(100.0f);
    dl->SetIntensity(3.0f);
    dl->SetShadowBias(0.003f);
    dl->SetCastShadows(true);
    dl->SetShadowSoftness(2);
    dl->SetShadowMapSize(Vector2i(1024));
    dlGo->GetTransform()->SetPosition(Vector3(5, 10, 10));
    dlGo->GetTransform()->LookAt(Vector3::Zero());
    dlGo->SetParent(this);

    p_modelContainer = GameObjectFactory::CreateGameObject();
    p_modelContainer->SetParent(this);
}

UvScene::~UvScene()
{
}

void UvScene::Update()
{
    Scene::Update();
}

void UvScene::OnModelChanged(Model *newModel)
{
    if (GameObject *previousModelGo = GetModelGameObject())
    {
        GameObject::Destroy(previousModelGo);
    }

    if (newModel)
    {
        GameObject *modelGo = newModel->CreateGameObjectFromModel();
        AABox modelAABox = modelGo->GetAABBoxWorld();
        float modelSize = modelAABox.GetSize().x;
        modelGo->GetTransform()->SetScale(Vector3(1.0f / modelSize));
        modelGo->GetTransform()->SetPosition(-modelAABox.GetCenter());
        modelGo->SetParent(p_modelContainer);
    }
}

GameObject *UvScene::GetModelGameObject() const
{
    GameObject *modelGo = p_modelContainer->GetChildren().Size() >= 1
                              ? p_modelContainer->GetChild(0)
                              : nullptr;
    return modelGo;
}
