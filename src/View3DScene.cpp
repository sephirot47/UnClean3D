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

View3DScene::~View3DScene()
{
}

void View3DScene::Update()
{
    GameObject::Update();

    if (Input::GetKeyDown(Key::A))
    {
        AddDirt();
    }

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

    Sphere goSphere = p_modelContainer->GetBoundingSphere();
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

void View3DScene::AddDirt()
{
    if (GameObject *modelGo = GetModelGameObject())
    {
        auto mrs = modelGo->GetComponentsInDescendantsAndThis<MeshRenderer>();
        for (MeshRenderer *mr : mrs)
        {
            if (Texture2D *originalAlbedoTex =
                    mr->GetMaterial()->GetAlbedoTexture())
            {
                Image originalAlbedoImg = originalAlbedoTex->ToImage();
                Image dirtImg = TextureFactory::GetSimplexNoiseTexture2D(
                                    originalAlbedoImg.GetSize())
                                    .Get()
                                    ->ToImage();
                Image finalAlbedoImg = originalAlbedoImg;

                for (uint y = 0; y < originalAlbedoImg.GetHeight(); ++y)
                {
                    for (uint x = 0; x < originalAlbedoImg.GetWidth(); ++x)
                    {
                        Color originalColor = originalAlbedoImg.GetPixel(x, y);
                        Color dirtColor = dirtImg.GetPixel(x, y);
                        Vector4 originalColorV = originalColor.ToVector4();
                        Vector4 dirtColorV = dirtColor.ToVector4();

                        Color finalColor =
                            Color::FromVector4(
                                Math::Clamp(originalColorV - dirtColorV,
                                            Vector4::Zero(),
                                            Vector4::One()))
                                .WithAlpha(originalColor.a);

                        finalAlbedoImg.SetPixel(x, y, finalColor);
                    }
                }

                AH<Texture2D> finalAlbedoTex = Assets::Create<Texture2D>();
                finalAlbedoTex.Get()->Import(finalAlbedoImg);

                mr->GetMaterial()->SetAlbedoTexture(finalAlbedoTex.Get());

                // finalAlbedoImg.Export(Path("finalAlbedoImg.png"));
            }
        }
    }
}

void View3DScene::OnModelChanged(Model *newModel)
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

        ResetCamera();
    }
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
    m_cameraOrbitPoint = p_modelContainer->GetBoundingSphere().GetCenter();
}
