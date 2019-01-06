#include "View3DScene.h"

#include "Bang/Assets.h"
#include "Bang/DirectionalLight.h"
#include "Bang/GEngine.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Input.h"
#include "Bang/Material.h"
#include "Bang/MeshRenderer.h"
#include "Bang/PointLight.h"
#include "Bang/TextureFactory.h"
#include "Bang/Transform.h"

#include "MainScene.h"

using namespace Bang;

View3DScene::View3DScene()
{
    AddComponent<Transform>();

    GameObject *camGo = GameObjectFactory::CreateGameObject();
    p_cam = GameObjectFactory::CreateDefaultCameraInto(camGo);
    p_cam->SetZFar(100.0f);
    p_cam->SetHDR(false);
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

    ResetCamera();
}

View3DScene::~View3DScene()
{
}

void View3DScene::Update()
{
    GameObject::Update();

    m_fpsChrono.MarkEnd();
    m_fpsChrono.MarkBegin();
    // Debug_DPeek(m_fpsChrono.GetMeanFPS());

    if (Input::IsMouseInsideContext() &&
        Input::GetMouseButtonDown(MouseButton::LEFT))
    {
        m_orbiting = true;
    }

    if (!Input::GetMouseButton(MouseButton::LEFT))
    {
        m_orbiting = false;
    }

    Transform *camTR = p_cam->GetGameObject()->GetTransform();
    if (m_orbiting)
    {
        Vector2 mouseCurrentAxisMovement = Input::GetMouseAxis();
        m_currentCameraRotAngles += mouseCurrentAxisMovement * 360.0f;

        m_currentCameraRotAngles.y =
            Math::Clamp(m_currentCameraRotAngles.y, -80.0f, 80.0f);
    }

    m_currentCameraZoom +=
        (-Input::GetMouseWheel().y * 0.1f) * m_currentCameraZoom;

    Sphere goSphere = p_modelContainer->GetBoundingSphereWorld();
    if (goSphere.GetRadius() > 0.0f)
    {
        float halfFov = Math::DegToRad(p_cam->GetFovDegrees() / 2.0f);
        float camDist = goSphere.GetRadius() / Math::Tan(halfFov) * 1.1f;
        camDist *= m_currentCameraZoom;
        Vector3 camDir =
            (Quaternion::AngleAxis(Math::DegToRad(-m_currentCameraRotAngles.x),
                                   Vector3::Up()) *
             Quaternion::AngleAxis(Math::DegToRad(m_currentCameraRotAngles.y),
                                   Vector3::Right()) *
             Vector3(0, 0, 1))
                .Normalized();
        Vector3 orbitPoint = m_cameraOrbitPoint;
        camTR->SetPosition(orbitPoint + camDir * camDist);
        camTR->LookAt(orbitPoint);

        p_cam->SetZNear(0.01f);
        p_cam->SetZFar((camDist + goSphere.GetRadius() * 2.0f) * 1.2f);
    }
}

void View3DScene::OnModelChanged(Model *newModel)
{
    Model *previousModel = p_currentModel;
    if (GameObject *previousModelGo = GetModelGameObject())
    {
        GameObject::Destroy(previousModelGo);
    }

    if (newModel)
    {
        GameObject *modelGo = newModel->CreateGameObjectFromModel();
        float modelSizeMax = modelGo->GetAABBoxWorld().GetSize().GetMax();
        modelGo->GetTransform()->SetLocalScale(
            modelGo->GetTransform()->GetLocalScale() *
            Vector3(1.0f / modelSizeMax));
        modelGo->GetTransform()->SetPosition(
            -modelGo->GetAABBoxWorld().GetCenter());
        modelGo->SetParent(p_modelContainer);

        Path prevModelPath =
            (previousModel ? previousModel->GetAssetFilepath() : Path::Empty());
        Path newModelPath = newModel->GetAssetFilepath();
        if (prevModelPath != newModelPath)
        {
            ResetCamera();
        }
    }

    p_currentModel = newModel;
}

Camera *View3DScene::GetCamera() const
{
    return p_cam;
}

GameObject *View3DScene::GetModelGameObject() const
{
    GameObject *modelGo = p_modelContainer->GetChildren().Size() >= 1
                              ? p_modelContainer->GetChild(0)
                              : nullptr;
    return modelGo;
}

Model *View3DScene::GetCurrentModel() const
{
    return MainScene::GetInstance()->GetCurrentModel();
}

void View3DScene::ResetCamera()
{
    m_currentCameraZoom = 1.4f;
    m_currentCameraRotAngles = Vector2(45.0f, -45.0f);
    m_cameraOrbitPoint = p_modelContainer->GetBoundingSphereWorld().GetCenter();
}
