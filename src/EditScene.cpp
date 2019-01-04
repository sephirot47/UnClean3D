#include "EditScene.h"

#include "Bang/Assets.h"
#include "Bang/GEngine.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Input.h"
#include "Bang/Material.h"
#include "Bang/MeshRenderer.h"
#include "Bang/TextureFactory.h"
#include "Bang/Transform.h"

using namespace Bang;

EditScene::EditScene()
{
    AddComponent<Transform>();

    GameObject *camGo = GameObjectFactory::CreateGameObject();
    p_cam = GameObjectFactory::CreateDefaultCameraInto(camGo);
    camGo->SetParent(this);
    SetCamera(p_cam);

    p_modelContainer = GameObjectFactory::CreateGameObject();
    p_modelContainer->SetParent(this);
}

EditScene::~EditScene()
{
}

void EditScene::Update()
{
    GameObject::Update();

    if (Input::GetKeyDown(Key::A))
    {
        AddDirt();
    }
    else if (Input::GetKeyDown(Key::R))
    {
        ResetModel();
    }

    Transform *camTR = p_cam->GetGameObject()->GetTransform();
    {
        if (Input::GetMouseButton(MouseButton::LEFT))
        {
            Vector2 mouseCurrentAxisMovement = Input::GetMouseAxis();
            m_currentCameraRotAngles += mouseCurrentAxisMovement * 360.0f;

            m_currentCameraRotAngles.y =
                Math::Clamp(m_currentCameraRotAngles.y, -80.0f, 80.0f);
        }

        m_currentCameraZoom +=
            (-Input::GetMouseWheel().y * 0.1f) * m_currentCameraZoom;
    }

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

void EditScene::LoadModel(const Path &modelPath, bool resetCamera)
{
    p_currentModel = Assets::Load<Model>(modelPath);
    if (p_currentModel)
    {
        if (GameObject *previousModelGo = GetModelGameObject())
        {
            GameObject::Destroy(previousModelGo);
        }

        GameObject *modelGo = p_currentModel.Get()->CreateGameObjectFromModel();
        modelGo->SetParent(p_modelContainer);

        if (resetCamera)
        {
            ResetCamera();
        }
    }
}

void EditScene::RenderScene(const Vector2i &renderSize)
{
    // Render camera
    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Push(GL::Pushable::VIEWPORT);

    p_cam->SetRenderSize(renderSize);
    GEngine::GetInstance()->RenderToGBuffer(this, p_cam);

    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
}

void EditScene::AddDirt()
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

void EditScene::ResetModel()
{
    if (p_currentModel)
    {
        LoadModel(p_currentModel.Get()->GetAssetFilepath(), false);
    }
}

Camera *EditScene::GetCamera() const
{
    return p_cam;
}

GameObject *EditScene::GetModelGameObject() const
{
    GameObject *modelGo = p_modelContainer->GetChildren().Size() >= 1
                              ? p_modelContainer->GetChild(0)
                              : nullptr;
    return modelGo;
}

void EditScene::ResetCamera()
{
    m_currentCameraZoom = 1.4f;
    m_currentCameraRotAngles = Vector2(45.0f, -45.0f);
    m_cameraOrbitPoint = p_modelContainer->GetBoundingSphere().GetCenter();
}
