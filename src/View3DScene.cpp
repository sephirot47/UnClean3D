#include "View3DScene.h"

#include "Bang/Assets.h"
#include "Bang/Camera.h"
#include "Bang/DirectionalLight.h"
#include "Bang/Framebuffer.h"
#include "Bang/GBuffer.h"
#include "Bang/GEngine.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Geometry.h"
#include "Bang/Input.h"
#include "Bang/LineRenderer.h"
#include "Bang/Material.h"
#include "Bang/MeshFactory.h"
#include "Bang/MeshRenderer.h"
#include "Bang/Model.h"
#include "Bang/Paths.h"
#include "Bang/Plane.h"
#include "Bang/PointLight.h"
#include "Bang/Shader.h"
#include "Bang/ShaderProgram.h"
#include "Bang/ShaderProgramFactory.h"
#include "Bang/TextureFactory.h"
#include "Bang/Transform.h"

#include "ControlPanel.h"
#include "EffectLayerCompositer.h"
#include "EffectLayerImplementation.h"
#include "MainScene.h"

using namespace Bang;

View3DScene::View3DScene()
{
    AddComponent<Transform>();

    // Camera
    GameObject *camGo = GameObjectFactory::CreateGameObject();
    p_cam = GameObjectFactory::CreateDefaultCameraInto(camGo);
    p_cam->SetZFar(100.0f);
    p_cam->SetHDR(true);
    camGo->SetParent(this);
    SetCamera(p_cam);

    // Init mask brush
    GameObject *maskBrushRendGo = GameObjectFactory::CreateGameObject();
    {
        p_maskBrushRend = maskBrushRendGo->AddComponent<LineRenderer>();
        Array<Vector3> circlePoints;
        constexpr int CircleSegments = 100;
        constexpr float step = (1.0f / CircleSegments);
        for (uint i = 0; i < CircleSegments; ++i)
        {
            float angle = (2.0f * Math::Pi) * i * step;
            Vector3 point(Math::Cos(angle), Math::Sin(angle), 0);
            circlePoints.PushBack(point);
        }

        for (uint i = 0; i < CircleSegments + 1; ++i)
        {
            p_maskBrushRend->SetPoint(i * 2, circlePoints[i % CircleSegments]);
            p_maskBrushRend->SetPoint(i * 2 + 1,
                                      circlePoints[(i + 1) % CircleSegments]);
        }

        p_maskBrushRend->GetMaterial()
            ->GetShaderProgramProperties()
            .SetLineWidth(1.0f);
        p_maskBrushRend->GetMaterial()
            ->GetShaderProgramProperties()
            .SetRenderPass(RenderPass::CANVAS);
        p_maskBrushRend->SetViewProjMode(GL::ViewProjMode::CANVAS);
        p_maskBrushRend->GetMaterial()->SetAlbedoColor(Color::Red());
        p_maskBrushRend->GetMaterial()->SetReceivesLighting(false);

        maskBrushRendGo->SetParent(this);
    }

    // Light
    GameObject *dlGo = GameObjectFactory::CreateGameObject();
    DirectionalLight *dl = dlGo->AddComponent<DirectionalLight>();
    dl->SetShadowStrength(1.0f);
    dl->SetShadowDistance(100.0f);
    dl->SetIntensity(3.0f);
    dl->SetShadowBias(0.003f);
    dl->SetCastShadows(false);
    dl->SetShadowSoftness(2);
    dl->SetShadowMapSize(Vector2i(1024));
    dlGo->GetTransform()->SetPosition(Vector3(5, 10, 10));
    dlGo->GetTransform()->LookAt(Vector3::Zero());
    dlGo->SetParent(this);

    m_view3DShaderProgram.Set(ShaderProgramFactory::Get(
        Paths::GetProjectAssetsDir().Append("Shaders").Append(
            "View3DShader.bushader")));

    m_effectLayerCompositer = new EffectLayerCompositer();

    p_modelContainer = GameObjectFactory::CreateGameObject();
    p_modelContainer->SetParent(this);

    ResetCamera();
}

View3DScene::~View3DScene()
{
    delete m_effectLayerCompositer;
}

void View3DScene::Update()
{
    GameObject::Update();

    Transform *camTR = p_cam->GetGameObject()->GetTransform();
    m_fpsChrono.MarkEnd();
    m_fpsChrono.MarkBegin();
    // Debug_DPeek(m_fpsChrono.GetMeanFPS());

    // Generate needed textures
    if (GetControlPanel()->GetMaskBrushEnabled() &&
        Input::GetMouseButton(MouseButton::LEFT))
    {
        PaintMaskBrush();
        InvalidateTextures();
    }

    if (!m_validTextures)
    {
        if (Time::GetNow()
                .GetPassedTimeSince(m_lastTimeTexturesGenerated)
                .GetSeconds() > 0.25)
        {
            Array<EffectLayer *> allEffectLayers = GetAllEffectLayers();
            for (EffectLayer *effectLayer : allEffectLayers)
            {
                if (effectLayer->GetImplementation() &&
                    effectLayer->GetImplementation()
                        ->CanGenerateEffectTextureInRealTime())
                {
                    effectLayer->GenerateEffectTexture();
                }
            }

            CompositeTextures();

            m_lastTimeTexturesGenerated = Time::GetNow();
            m_validTextures = true;
        }
    }

    // Stuff needed for later
    ControlPanel *controlPanel = GetControlPanel();

    if (MainScene::GetInstance()->GetSceneMode() !=
        MainScene::SceneMode::VIEW3D)
    {
        return;
    }

    bool _;
    const Plane orbitPlane(m_cameraOrbitPoint, camTR->GetBack());
    const Vector2 currentMousePos(Input::GetMousePosition());
    const Ray currentMouseCamRay =
        GetCamera()->FromViewportPointNDCToRay(GL::FromPointToPointNDC(
            currentMousePos, Vector2(GL::GetViewportSize())));
    Vector3 mouseOrbitPlaneIntersection;
    Geometry::IntersectRayPlane(
        currentMouseCamRay, orbitPlane, &_, &mouseOrbitPlaneIntersection);

    // Get intersection of mouse ray with mesh
    bool intersectedMouseRayWithMesh = false;
    float closestMouseRayMeshIntersectionDistance = Math::Infinity<float>();
    Vector3 closestMouseRayMeshIntersectionPoint;
    for (const auto &it : m_meshRendererToInfo)
    {
        MeshRenderer *mr = it.first;
        const Matrix4 localToWorldMatrix =
            mr->GetGameObject()->GetTransform()->GetLocalToWorldMatrix();
        for (Mesh::TriangleId triId = 0;
             triId < mr->GetMesh()->GetNumTriangles();
             ++triId)
        {
            Triangle tri = mr->GetMesh()->GetTriangle(triId);
            tri = localToWorldMatrix * tri;
            bool intersectedLocal = false;
            float distance = 0.0f;
            Geometry::IntersectRayTriangle(
                currentMouseCamRay, tri, &intersectedLocal, &distance);
            if (intersectedLocal &&
                distance < closestMouseRayMeshIntersectionDistance)
            {
                intersectedMouseRayWithMesh = true;
                closestMouseRayMeshIntersectionDistance = distance;
                closestMouseRayMeshIntersectionPoint =
                    currentMouseCamRay.GetPoint(distance);
            }
        }
    }

    // Mask brush line rendering handling
    {
        GameObject *brushGo = p_maskBrushRend->GetGameObject();
        Transform *brushTR = brushGo->GetTransform();
        brushGo->SetEnabled(GetControlPanel()->GetMaskBrushEnabled());
        brushTR->SetPosition(Vector3(Input::GetMousePosition(), 0));
        brushTR->SetScale(controlPanel->GetMaskBrushSize());
    }

    // Camera movement
    {
        if (Input::IsMouseInsideContext() &&
            Input::GetMouseButtonDown(MouseButton::RIGHT))
        {
            m_orbiting = true;
        }

        if (!Input::GetMouseButton(MouseButton::RIGHT))
        {
            m_orbiting = false;
        }

        if (Input::GetMouseButton(MouseButton::MIDDLE))
        {
            Vector2 mouseDelta(Input::GetMouseDelta());
            Vector2 prevMousePos = currentMousePos - mouseDelta;

            Ray prevMouseCamRay =
                GetCamera()->FromViewportPointNDCToRay(GL::FromPointToPointNDC(
                    prevMousePos, Vector2(GL::GetViewportSize())));

            Vector3 prevIntersectionPoint;
            Geometry::IntersectRayPlane(
                prevMouseCamRay, orbitPlane, &_, &prevIntersectionPoint);

            Vector3 displacement =
                (mouseOrbitPlaneIntersection - prevIntersectionPoint);
            m_cameraOrbitPoint -= displacement;
        }

        if (Input::GetKeyDown(Key::F))
        {
            if (intersectedMouseRayWithMesh)
            {
                m_cameraOrbitPoint = closestMouseRayMeshIntersectionPoint;
            }
        }

        if (m_orbiting)
        {
            Vector2 mouseCurrentAxisMovement = Input::GetMouseAxis();
            m_currentCameraRotAngles += mouseCurrentAxisMovement * 360.0f;

            m_currentCameraRotAngles.y =
                Math::Clamp(m_currentCameraRotAngles.y, -80.0f, 80.0f);
        }

        if (!Input::GetKey(Key::LSHIFT))
        {
            m_currentCameraZoom +=
                (-Input::GetMouseWheel().y * 0.1f) * m_currentCameraZoom;
        }

        Sphere goSphere = p_modelContainer->GetBoundingSphereWorld();
        if (goSphere.GetRadius() > 0.0f)
        {
            float halfFov = Math::DegToRad(p_cam->GetFovDegrees() / 2.0f);
            float camDist = goSphere.GetRadius() / Math::Tan(halfFov) * 1.1f;
            camDist *= m_currentCameraZoom;
            Vector3 camDir = (Quaternion::AngleAxis(
                                  Math::DegToRad(-m_currentCameraRotAngles.x),
                                  Vector3::Up()) *
                              Quaternion::AngleAxis(
                                  Math::DegToRad(m_currentCameraRotAngles.y),
                                  Vector3::Right()) *
                              Vector3(0, 0, 1))
                                 .Normalized();
            Vector3 orbitPoint = m_cameraOrbitPoint;
            camTR->SetPosition(orbitPoint + camDir * camDist + m_cameraOffset);
            camTR->LookAt(orbitPoint);

            p_cam->SetZNear(0.01f);
            p_cam->SetZFar((camDist + goSphere.GetRadius() * 2.0f) * 1.2f);
        }
    }

    // Other shortcuts
    {
        if (Input::GetKeyDown(Key::S))
        {
            ReloadShaders();
        }
    }
}

void View3DScene::Render(RenderPass rp, bool renderChildren)
{
    if (rp == RenderPass::SCENE_OPAQUE)
    {
        ApplyControlPanelSettingsToModel();
        ApplyCompositeTexturesToModel();
        SetViewUniforms();
    }

    Scene::Render(rp, renderChildren);

    if (rp == RenderPass::SCENE_OPAQUE)
    {
        RestoreOriginalAlbedoTexturesToModel();
    }
}

void View3DScene::ReloadShaders()
{
    m_view3DShaderProgram.Get()->ReImport();
    m_effectLayerCompositer->ReloadShaders();
    for (auto &it : m_meshRendererToInfo)
    {
        const Array<EffectLayer *> &effectLayers = it.second.effectLayers;
        for (EffectLayer *effectLayer : effectLayers)
        {
            effectLayer->ReloadShaders();
        }
    }
    InvalidateTextures();
}

void View3DScene::OnModelChanged(Model *newModel)
{
    Model *previousModel = p_currentModel;
    if (GameObject *previousModelGo = GetModelGameObject())
    {
        GameObject::Destroy(previousModelGo);
        for (auto &it : m_meshRendererToInfo)
        {
            const Array<EffectLayer *> &effectLayers = it.second.effectLayers;
            for (EffectLayer *effectLayer : effectLayers)
            {
                delete effectLayer;
            }
        }
        m_meshRendererToInfo.Clear();
        ApplyControlPanelSettingsToModel();
        ApplyCompositeTexturesToModel();
        InvalidateTextures();
    }

    if (newModel)
    {
        GameObject *modelGo = newModel->CreateGameObjectFromModel();
        m_originalModelLocalScale = modelGo->GetTransform()->GetLocalScale();

        float modelSizeMax = modelGo->GetAABBoxWorld().GetSize().GetMax();
        modelGo->GetTransform()->SetLocalScale(
            modelGo->GetTransform()->GetLocalScale() *
            Vector3(1.0f / modelSizeMax));
        modelGo->GetTransform()->SetPosition(
            -modelGo->GetAABBoxWorld().GetCenter());
        modelGo->SetParent(p_modelContainer);

        Array<MeshRenderer *> mrs =
            p_modelContainer->GetComponentsInDescendantsAndThis<MeshRenderer>();
        for (MeshRenderer *mr : mrs)
        {
            Material *mat = mr->GetMaterial();

            // Create default textures if they do not exist
            if (!mat->GetAlbedoTexture())
            {
                constexpr int DTS = 1024;
                AH<Texture2D> defaultAlbedoTex = Assets::Create<Texture2D>();
                defaultAlbedoTex.Get()->Fill(Color::Red(), DTS, DTS);
                mat->SetAlbedoTexture(defaultAlbedoTex.Get());
            }

            const Vector2i albedoTexSize = mat->GetAlbedoTexture()->GetSize();
            const Vector2i &ats = albedoTexSize;
            if (!mat->GetNormalMapTexture())
            {
                AH<Texture2D> defaultNormalTex = Assets::Create<Texture2D>();
                defaultNormalTex.Get()->Fill(
                    Color(0.5f, 0.5f, 1.0f, 1.0f), ats.x, ats.y);
                mat->SetNormalMapTexture(defaultNormalTex.Get());
            }

            if (!mat->GetRoughnessTexture())
            {
                AH<Texture2D> defaultRoughnessTex = Assets::Create<Texture2D>();
                defaultRoughnessTex.Get()->Fill(Color(1.0f), ats.x, ats.y);
                mat->SetRoughnessTexture(defaultRoughnessTex.Get());
            }

            if (!mat->GetMetalnessTexture())
            {
                AH<Texture2D> defaultMetalnessTex = Assets::Create<Texture2D>();
                defaultMetalnessTex.Get()->Fill(Color(1.0f), ats.x, ats.y);
                mat->SetMetalnessTexture(defaultMetalnessTex.Get());
            }

            MeshRendererInfo mrInfo;
            mrInfo.originalAlbedoTexture =
                AH<Texture2D>(mat->GetAlbedoTexture());
            mrInfo.originalNormalTexture =
                AH<Texture2D>(mat->GetNormalMapTexture());
            mrInfo.originalRoughnessTexture =
                AH<Texture2D>(mat->GetRoughnessTexture());
            mrInfo.originalMetalnessTexture =
                AH<Texture2D>(mat->GetMetalnessTexture());
            m_meshRendererToInfo.Add(mr, mrInfo);

            mat->SetShaderProgram(m_view3DShaderProgram.Get());
        }

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

EffectLayer *View3DScene::CreateNewEffectLayer()
{
    EffectLayer *newEffectLayer = nullptr;
    if (m_meshRendererToInfo.Size() >= 1)
    {
        MeshRenderer *mr = m_meshRendererToInfo.Begin()->first;
        MeshRendererInfo &mrInfo = m_meshRendererToInfo.Begin()->second;
        Array<EffectLayer *> &effectLayers = mrInfo.effectLayers;

        newEffectLayer = new EffectLayer(mr);
        effectLayers.PushFront(newEffectLayer);
    }
    return newEffectLayer;
}

void View3DScene::RemoveEffectLayer(uint effectLayerIdx)
{
    for (auto &it : m_meshRendererToInfo)
    {
        MeshRenderer *mr = it.first;
        Array<EffectLayer *> &effectLayers = it.second.effectLayers;
        effectLayers.RemoveByIndex(effectLayerIdx);
    }
    InvalidateTextures();
}

void View3DScene::ApplyControlPanelSettingsToModel()
{
    ControlPanel *cpanel = GetControlPanel();
    for (const auto &it : m_meshRendererToInfo)
    {
        MeshRenderer *mr = it.first;
        mr->GetMaterial()->SetRoughness(1.0f);
        mr->GetMaterial()->SetMetalness(1.0f);
    }
}

void View3DScene::ApplyCompositeTexturesToModel()
{
    for (const auto &it : m_meshRendererToInfo)
    {
        MeshRenderer *mr = it.first;
        const MeshRendererInfo &mrInfo = it.second;
        if (Material *mat = mr->GetMaterial())
        {
            EffectLayerCompositer *compositer = GetEffectLayerCompositer();
            mat->SetAlbedoTexture(compositer->GetFinalAlbedoTexture());
            mat->SetNormalMapTexture(compositer->GetFinalNormalTexture());
            mat->SetRoughnessTexture(compositer->GetFinalRoughnessTexture());
            mat->SetMetalnessTexture(compositer->GetFinalMetalnessTexture());
        }
    }
}

void View3DScene::SetViewUniforms()
{
    GetControlPanel()->SetControlPanelUniforms(m_view3DShaderProgram.Get());
}

void View3DScene::PaintMaskBrush()
{
    Array<EffectLayer *> selectedEffectLayers = GetSelectedEffectLayers();
    for (EffectLayer *selectedEffectLayer : selectedEffectLayers)
    {
        selectedEffectLayer->PaintMaskBrush();
    }
}

void View3DScene::CompositeTextures()
{
    for (const auto &it : m_meshRendererToInfo)
    {
        MeshRenderer *mr = it.first;
        const MeshRendererInfo &mrInfo = it.second;
        if (Material *mat = mr->GetMaterial())
        {
            const Array<EffectLayer *> &effectLayers = mrInfo.effectLayers;
            EffectLayerCompositer *compositer = GetEffectLayerCompositer();
            compositer->CompositeLayers(effectLayers,
                                        mrInfo.originalAlbedoTexture.Get(),
                                        mrInfo.originalNormalTexture.Get(),
                                        mrInfo.originalRoughnessTexture.Get(),
                                        mrInfo.originalMetalnessTexture.Get());
        }
    }
}

void View3DScene::InvalidateTextures()
{
    m_validTextures = false;
}

void View3DScene::RestoreOriginalAlbedoTexturesToModel()
{
    for (const auto &it : m_meshRendererToInfo)
    {
        MeshRenderer *mr = it.first;
        if (Material *mat = mr->GetMaterial())
        {
            const MeshRendererInfo &mrInfo = it.second;
            mat->SetAlbedoTexture(mrInfo.originalAlbedoTexture.Get());
            mat->SetNormalMapTexture(mrInfo.originalNormalTexture.Get());
            mat->SetRoughnessTexture(mrInfo.originalRoughnessTexture.Get());
            mat->SetMetalnessTexture(mrInfo.originalMetalnessTexture.Get());
        }
    }
}

void View3DScene::MoveEffectLayer(EffectLayer *effectLayer, uint newIndex)
{
    for (auto &pair : m_meshRendererToInfo)
    {
        MeshRendererInfo &mrInfo = pair.second;
        mrInfo.effectLayers.Remove(effectLayer);
        mrInfo.effectLayers.Insert(effectLayer, newIndex);
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

const Vector3 &View3DScene::GetModelOriginalLocalScale() const
{
    return m_originalModelLocalScale;
}

Array<EffectLayer *> View3DScene::GetAllEffectLayers() const
{
    Array<EffectLayer *> effectLayers;
    for (auto &it : m_meshRendererToInfo)
    {
        const MeshRendererInfo &mrInfo = it.second;
        effectLayers.PushBack(mrInfo.effectLayers);
    }
    return effectLayers;
}

Array<EffectLayer *> View3DScene::GetSelectedEffectLayers() const
{
    Array<EffectLayer *> effectLayers;
    for (auto &it : m_meshRendererToInfo)
    {
        const MeshRendererInfo &mrInfo = it.second;
        const Array<EffectLayer *> &mrEffectLayers = mrInfo.effectLayers;
        uint selIdx = GetControlPanel()->GetSelectedUIEffectLayerIndex();
        if (selIdx < mrEffectLayers.Size())
        {
            EffectLayer *efl = mrEffectLayers[selIdx];
            effectLayers.PushBack(efl);
        }
    }
    return effectLayers;
}

EffectLayerCompositer *View3DScene::GetEffectLayerCompositer() const
{
    return m_effectLayerCompositer;
}

Model *View3DScene::GetCurrentModel() const
{
    return MainScene::GetInstance()->GetCurrentModel();
}

void View3DScene::ResetCamera()
{
    m_cameraOffset = Vector3::Zero();
    m_currentCameraZoom = 1.4f;
    m_currentCameraRotAngles = Vector2(45.0f, -45.0f);
    m_cameraOrbitPoint = p_modelContainer->GetBoundingSphereWorld().GetCenter();
}

ControlPanel *View3DScene::GetControlPanel() const
{
    return MainScene::GetInstance()->GetControlPanel();
}
