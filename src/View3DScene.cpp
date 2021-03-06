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
#include "Bang/Thread.h"
#include "Bang/Transform.h"
#include "Bang/VAO.h"
#include "Bang/VBO.h"

#include "ControlPanel.h"
#include "EffectLayerCompositer.h"
#include "EffectLayerMask.h"
#include "EffectLayerMaskImplementation.h"
#include "EffectLayerMaskImplementationBrush.h"
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

    // Load skyboxes
    {
        m_openSeaSS =
            Assets::Load<Texture2D>(Paths::GetEngineAssetsDir().Append(
                "Textures/DefaultSkyboxBack.jpg"));

        auto LoadCM = [](
            AH<TextureCubeMap> *cm, AH<Texture2D> *ss, const String &dirName) {
            Path sbsDir =
                Paths::GetProjectAssetsDir().Append("Skyboxes").Append(dirName);
            Image rightImg, leftImg, topImg, botImg, backImg, frontImg;
            *cm = Assets::Create<TextureCubeMap>();
            {
                rightImg.Import(sbsDir.Append("posx.jpg"));
                leftImg.Import(sbsDir.Append("negx.jpg"));
                topImg.Import(sbsDir.Append("posy.jpg"));
                botImg.Import(sbsDir.Append("negy.jpg"));
                backImg.Import(sbsDir.Append("posz.jpg"));
                frontImg.Import(sbsDir.Append("negz.jpg"));

                cm->Get()->Import(
                    rightImg, leftImg, topImg, botImg, frontImg, backImg);

                *ss = Assets::Create<Texture2D>();
                ss->Get()->Import(leftImg);
            }
        };
        LoadCM(&m_yokohamaCM, &m_yokohamaSS, "Yokohama");
        LoadCM(&m_parkCM, &m_parkSS, "Park");
        LoadCM(&m_hotelCM, &m_hotelSS, "Hotel");
    }

    // Init mask brush and preview related stuff
    GameObject *maskBrushPreviewRendererGo =
        GameObjectFactory::CreateGameObject();
    {
        p_maskBrushPreviewRenderer =
            maskBrushPreviewRendererGo->AddComponent<MeshRenderer>();
        p_maskBrushPreviewRenderer->SetMesh(MeshFactory::GetPlane().Get());

        m_brushPreviewSP.Set(ShaderProgramFactory::Get(
            Paths::GetProjectAssetsDir().Append("Shaders").Append(
                "BrushPreview.bushader")));
        p_maskBrushPreviewRenderer->GetMaterial()->SetShaderProgram(
            m_brushPreviewSP.Get());

        p_maskBrushPreviewRenderer->GetMaterial()
            ->GetShaderProgramProperties()
            .SetLineWidth(1.0f);
        p_maskBrushPreviewRenderer->GetMaterial()
            ->GetShaderProgramProperties()
            .SetRenderPass(RenderPass::CANVAS);
        p_maskBrushPreviewRenderer->SetViewProjMode(GL::ViewProjMode::CANVAS);
        p_maskBrushPreviewRenderer->GetMaterial()->SetAlbedoColor(Color::White());
        p_maskBrushPreviewRenderer->GetMaterial()->SetReceivesLighting(false);

        maskBrushPreviewRendererGo->GetTransform()->SetLocalScale(
            Vector3(0.3f));
        maskBrushPreviewRendererGo->SetParent(this);
    }

    m_glslRayCaster = new GLSLRayCaster();

    // Light
    GameObject *dlGo = GameObjectFactory::CreateGameObject();
    p_dirLight = dlGo->AddComponent<DirectionalLight>();
    p_dirLight->SetShadowStrength(1.0f);
    p_dirLight->SetShadowDistance(100.0f);
    p_dirLight->SetIntensity(3.0f);
    p_dirLight->SetShadowBias(0.003f);
    p_dirLight->SetCastShadows(true);
    p_dirLight->SetShadowSoftness(2);
    p_dirLight->SetShadowMapSize(Vector2i(1024));
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

    Array<EffectLayer *> allEffectLayers = GetAllEffectLayers();
    for (EffectLayer *effectLayer : allEffectLayers)
    {
        for (EffectLayerMask *mask : effectLayer->GetMasks())
        {
            mask->Update();
        }
    }

    if (Time::GetNow()
            .GetPassedTimeSince(m_lastTimeTexturesValidated)
            .GetSeconds() > 0.25)
    {
        bool allLayersValid = true;
        for (EffectLayer *effectLayer : allEffectLayers)
        {
            if (!effectLayer->IsValid())
            {
                allLayersValid = false;
                effectLayer->GenerateEffectTexture();
            }
        }

        if (!allLayersValid)
        {
            CompositeTextures();
        }

        m_lastTimeTexturesValidated = Time::GetNow();
    }

    if (Input::GetKeyDown(Key::S))
    {
        ReloadShaders();
    }

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
    if (GetControlPanel()->GetMaskBrushEnabled())
    {
        GameObject *brushGo = p_maskBrushPreviewRenderer->GetGameObject();
        Transform *brushTR = brushGo->GetTransform();
        brushGo->SetEnabled(GetControlPanel()->GetMaskBrushEnabled());
        brushTR->SetPosition(Vector3(Input::GetMousePosition(), 0));
        EffectLayerMaskImplementationBrush *maskBrush =
            DCAST<EffectLayerMaskImplementationBrush *>(
                GetControlPanel()
                    ->GetSelectedEffectLayerMask()
                    ->GetImplementation());
        brushTR->SetScale(maskBrush->GetBrushSize());
    }
    p_maskBrushPreviewRenderer->SetEnabled(
        GetControlPanel()->GetMaskBrushEnabled());

    // Camera movement
    const Sphere goSphere = p_modelContainer->GetBoundingSphereWorld();
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

    // Some scene options
    {
        if (GetControlPanel()->GetRotateLights())
        {
            float angleDeltaX = Time::GetDeltaTime().GetSeconds() * 0.5f;
            float angleDeltaY = Time::GetDeltaTime().GetSeconds() * 0.7f;
            p_dirLight->GetGameObject()->GetTransform()->RotateLocal(
                Quaternion::AngleAxis(angleDeltaY, Vector3::Up()));
            p_dirLight->GetGameObject()->GetTransform()->RotateLocal(
                Quaternion::AngleAxis(angleDeltaX, Vector3::Right()));
        }

        for (auto &it : m_meshRendererToInfo)
        {
            MeshRenderer *mr = it.first;
            mr->SetCastsShadows( GetControlPanel()->GetShadows() );
            mr->SetReceivesShadows( GetControlPanel()->GetShadows() );
        }
    }

    Thread::SleepCurrentThread(0.033f);
}

void View3DScene::Render(RenderPass rp, bool renderChildren)
{
    if (rp == RenderPass::SCENE_OPAQUE)
    {
        ApplyControlPanelSettingsToModel();
        ApplyCompositeTexturesToModel();

        GetControlPanel()->SetControlPanelUniforms(m_brushPreviewSP.Get());
        GetControlPanel()->SetControlPanelUniforms(m_view3DShaderProgram.Get());
    }

    Scene::Render(rp, renderChildren);

    if (rp == RenderPass::SCENE_OPAQUE)
    {
        RestoreOriginalTexturesToModel();
    }
}

void View3DScene::ReloadShaders()
{
    m_brushPreviewSP.Get()->ReImport();
    m_view3DShaderProgram.Get()->ReImport();
    m_effectLayerCompositer->ReloadShaders();
    GetControlPanel()->ReloadShaders();
    for (auto &it : m_meshRendererToInfo)
    {
        const Array<EffectLayer *> &effectLayers = it.second.effectLayers;
        for (EffectLayer *effectLayer : effectLayers)
        {
            effectLayer->ReloadShaders();
        }
    }
}

void View3DScene::OnModelChanged(Model *newModel)
{
    Model *previousModel = GetCurrentModel();
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
    }
    m_meshRendererToInfo.Clear();

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
            Mesh *mesh = mr->GetMesh();
            if (!mesh || mesh->GetPositionsPool().Size() == 0)
            {
                continue;
            }

            Material *mat = mr->GetMaterial();
            bool hadAlbedoTexture = mat->GetAlbedoTexture();
            bool hadNormalMapTexture = mat->GetNormalMapTexture();
            bool hadRoughnessTexture = mat->GetRoughnessTexture();
            bool hadMetalnessTexture = mat->GetMetalnessTexture();

            // Create default textures if they do not exist
            if (!mat->GetAlbedoTexture())
            {
                AH<Texture2D> defaultAlbedoTex = Assets::Create<Texture2D>();
                mat->SetAlbedoTexture(defaultAlbedoTex.Get());
            }

            if (!mat->GetNormalMapTexture())
            {
                AH<Texture2D> defaultNormalTex = Assets::Create<Texture2D>();
                mat->SetNormalMapTexture(defaultNormalTex.Get());
            }

            if (!mat->GetRoughnessTexture())
            {
                AH<Texture2D> defaultRoughnessTex = Assets::Create<Texture2D>();
                mat->SetRoughnessTexture(defaultRoughnessTex.Get());
            }

            if (!mat->GetMetalnessTexture())
            {
                AH<Texture2D> defaultMetalnessTex = Assets::Create<Texture2D>();
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

            mrInfo.originalAlbedoTexture.Get()->SetFormat(GL::ColorFormat::RGBA16F);
            mrInfo.originalNormalTexture.Get()->SetFormat(GL::ColorFormat::RGBA32F);
            mrInfo.originalRoughnessTexture.Get()->SetFormat(GL::ColorFormat::R16F);
            mrInfo.originalMetalnessTexture.Get()->SetFormat(GL::ColorFormat::R16F);

            if (!hadAlbedoTexture)
            {
                mat->GetAlbedoTexture()->Fill(Color::White(), 32, 32);
            }

            if (!hadNormalMapTexture)
            {
                mat->GetNormalMapTexture()->Fill(Color(0.5f, 0.5f, 1.0f, 1.0f), 1, 1);
            }

            if (!hadRoughnessTexture)
            {
                mat->GetRoughnessTexture()->Fill(Color(1.0f), 1, 1);
            }

            if (!hadMetalnessTexture)
            {
                mat->GetMetalnessTexture()->Fill(Color(1.0f), 1, 1);
            }

            mat->SetShaderProgram(m_view3DShaderProgram.Get());

            m_glslRayCaster->SetMeshRenderer(mr);

            // Create texture mesh
            {
                AH<Mesh> textureMeshAH = Assets::Create<Mesh>();
                Mesh *textureMesh = textureMeshAH.Get();

                // Gather some data
                Array<Vector3> texTriMeshPositions;
                Array<Vector3> originalVertexPositions;
                Array<Vector3> originalVertexNormals;
                Array<float> originalTriIds;
                for (Mesh::VertexId triId = 0; triId < mesh->GetNumTriangles();
                     ++triId)
                {
                    for (uint i = 0; i < 3; ++i)
                    {
                        Mesh::VertexId vId =
                            mesh->GetTrianglesVertexIds()[triId * 3 + i];
                        if (vId >= mesh->GetUvsPool().Size())
                        {
                            break;
                        }

                        const Vector2 &oriVertUv = mesh->GetUvsPool()[vId];
                        Vector3 texTriMeshPos = oriVertUv.xy0() * 2.0f - 1.0f;
                        texTriMeshPos.y *= -1;
                        texTriMeshPositions.PushBack(texTriMeshPos);

                        const Vector3 &oriVertPos =
                            mesh->GetPositionsPool()[vId];
                        const Vector3 &oriVertNormal =
                            mesh->GetNormalsPool()[vId];
                        originalVertexPositions.PushBack(oriVertPos);
                        originalVertexNormals.PushBack(oriVertNormal);
                        originalTriIds.PushBack(triId);
                    }
                }

                // Set original mesh uvs as texture mesh positions
                textureMesh->SetPositionsPool(texTriMeshPositions);
                textureMesh->SetTrianglesVertexIds({});

                // Add actual original mesh attributes as other VBOs
                VBO *positionsVBO = new VBO();
                positionsVBO->CreateAndFill(
                    originalVertexPositions.Data(),
                    originalVertexPositions.Size() * sizeof(float) * 3);
                textureMesh->GetVAO()->SetVBO(
                    positionsVBO, 1, 3, GL::VertexAttribDataType::FLOAT);

                VBO *normalsVBO = new VBO();
                normalsVBO->CreateAndFill(
                    originalVertexNormals.Data(),
                    originalVertexNormals.Size() * sizeof(float) * 3);
                textureMesh->GetVAO()->SetVBO(
                    normalsVBO, 2, 3, GL::VertexAttribDataType::FLOAT);

                VBO *triIdsVBO = new VBO();
                triIdsVBO->CreateAndFill(originalTriIds.Data(),
                                         originalTriIds.Size() * sizeof(float));
                textureMesh->GetVAO()->SetVBO(
                    triIdsVBO, 3, 1, GL::VertexAttribDataType::FLOAT);

                textureMesh->UpdateVAOs();

                m_textureMesh.Set(textureMesh);
            }

            mesh->UpdateCornerTablesIfNeeded();
        }

        Path prevModelPath =
            (previousModel ? previousModel->GetAssetFilepath() : Path::Empty());
        Path newModelPath = newModel->GetAssetFilepath();
        if (prevModelPath != newModelPath)
        {
            ResetCamera();
        }
    }
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
        Array<EffectLayer *> &effectLayers = it.second.effectLayers;
        effectLayers.RemoveByIndex(effectLayerIdx);
    }

    InvalidateAll();
    if (GetAllEffectLayers().IsEmpty())
    {
        CompositeTextures();
    }
}

void View3DScene::ApplyControlPanelSettingsToModel()
{
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

void View3DScene::RestoreOriginalTexturesToModel()
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
    InvalidateAll();
}

void View3DScene::SetEnvironment(View3DScene::Environment environment)
{
    m_environment = environment;
    AH<TextureCubeMap> cm;
    switch (environment)
    {
        case Environment::YOKOHAMA_NIGHT: cm = m_yokohamaCM; break;
        case Environment::PARK: cm = m_parkCM; break;
        case Environment::HOTEL: cm = m_hotelCM; break;
        default: cm.Set(TextureFactory::GetDefaultSkybox());
    }
    p_cam->SetSkyBoxTexture(cm.Get());
}

void View3DScene::RegenerateAllEffectLayers()
{
    auto effectLayers = GetAllEffectLayers();
    for (auto &effectLayer : effectLayers)
    {
        effectLayer->GenerateEffectTexture(true);
    }
    CompositeTextures();
}

void View3DScene::InvalidateAll()
{
    Array<EffectLayer *> effectLayers = GetAllEffectLayers();
    for (EffectLayer *effectLayer : effectLayers)
    {
        effectLayer->Invalidate(true);
    }
}

Camera *View3DScene::GetCamera() const
{
    return p_cam;
}

GLSLRayCaster *View3DScene::GetGLSLRayCaster() const
{
    return m_glslRayCaster;
}

GameObject *View3DScene::GetModelGameObject() const
{
    GameObject *modelGo = p_modelContainer->GetChildren().Size() >= 1
                              ? p_modelContainer->GetChild(0)
                              : nullptr;
    return modelGo;
}

View3DScene::Environment View3DScene::GetEnvironment() const
{
    return m_environment;
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

Array<EffectLayerMask *> View3DScene::GetSelectedEffectLayerMasks() const
{
    Array<EffectLayerMask *> effectLayerMasks;
    for (auto &it : m_meshRendererToInfo)
    {
        const MeshRendererInfo &mrInfo = it.second;
        const Array<EffectLayer *> &mrEffectLayers = mrInfo.effectLayers;
        uint elSelIdx = GetControlPanel()->GetSelectedUIEffectLayerIndex();
        if (elSelIdx < mrEffectLayers.Size())
        {
            EffectLayer *efl = mrEffectLayers[elSelIdx];
            uint maskSelectedIdx =
                GetControlPanel()->GetSelectedUIEffectLayerMaskIndex();
            if (maskSelectedIdx != -1u)
            {
                EffectLayerMask *eflMask = efl->GetMasks()[maskSelectedIdx];
                effectLayerMasks.PushBack(eflMask);
            }
        }
    }
    return effectLayerMasks;
}

EffectLayerCompositer *View3DScene::GetEffectLayerCompositer() const
{
    return m_effectLayerCompositer;
}

Texture2D *View3DScene::GetEnvironmentSnapshot(
    View3DScene::Environment environment) const
{
    switch (environment)
    {
        case Environment::YOKOHAMA_NIGHT: return m_yokohamaSS.Get();
        case Environment::PARK: return m_parkSS.Get();
        case Environment::HOTEL: return m_hotelSS.Get();
        default: break;
    }
    return m_openSeaSS.Get();
}

Model *View3DScene::GetCurrentModel() const
{
    return MainScene::GetInstance()->GetCurrentModel();
}

Mesh *View3DScene::GetTextureMesh() const
{
    return m_textureMesh.Get();
}

EffectLayerCompositer *View3DScene::GetCompositer() const
{
    return m_effectLayerCompositer;
}

View3DScene *View3DScene::GetInstance()
{
    return MainScene::GetInstance()->GetView3DScene();
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
