#include "MainScene.h"

#include "Bang/Assets.h"
#include "Bang/Camera.h"
#include "Bang/GBuffer.h"
#include "Bang/GEngine.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Input.h"
#include "Bang/RectTransform.h"
#include "Bang/UICanvas.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"

#include "ControlPanel.h"
#include "Dirter.h"
#include "SceneImage.h"
#include "TexturesScene.h"
#include "UvScene.h"
#include "View3DScene.h"

using namespace Bang;

MainScene *MainScene::s_mainScene = nullptr;

MainScene::MainScene()
{
    MainScene::s_mainScene = this;

    GameObjectFactory::CreateUISceneInto(this);
    UIHorizontalLayout *hl = AddComponent<UIHorizontalLayout>();
    p_view3DScene = new View3DScene();
    p_uvScene = new UvScene();
    p_texturesScene = new TexturesScene();

    p_sceneImage = new SceneImage();
    GetSceneImage()->SetParent(this);

    p_controlPanel = new ControlPanel();
    GetControlPanel()->SetParent(this);

    GetControlPanel()->OpenModel(
        Path("/home/sephirot47/Downloads/MyTable/MyTable.dae"));
}

MainScene::~MainScene()
{
}

void MainScene::Update()
{
    Scene::Update();

    if (Input::GetKeyDown(Key::R))
    {
        ResetModel();
    }
    else if (Input::GetKeyDown(Key::A))
    {
        Dirter::AddDirt(GetCurrentModelGameObject());
    }
    else if (Input::GetKeyDown(Key::V))
    {
        SetSceneMode(MainScene::SceneMode::VIEW3D);
    }
    else if (Input::GetKeyDown(Key::U))
    {
        SetSceneMode(MainScene::SceneMode::UV);
    }
    else if (Input::GetKeyDown(Key::T))
    {
        SetSceneMode(MainScene::SceneMode::TEXTURES);
    }
}

void MainScene::Render(RenderPass renderPass, bool renderChildren)
{
    if (renderPass == RenderPass::CANVAS)
    {
        Input::Context prevContext = Input::GetContext();

        AARecti renderRect(
            GetSceneImage()->GetRectTransform()->GetViewportAARect());
        Vector2i renderSize(renderRect.GetSize());

        Input::Context sceneContext;
        sceneContext.rect = renderRect;
        Input::SetContext(sceneContext);

        Scene *scene = GetCurrentRenderScene();
        scene->GetCamera()->SetRenderSize(renderSize);
        GL::SetViewport(renderRect);

        GetCurrentRenderScene()->Update();
        GEngine::GetInstance()->Render(scene, scene->GetCamera());

        GetSceneImage()->SetImageTexture(
            scene->GetCamera()->GetGBuffer()->GetDrawColorTexture());

        Input::SetContext(prevContext);
    }

    Scene::Render(renderPass, renderChildren);
}

void MainScene::ResetModel()
{
    if (GetCurrentModel())
    {
        LoadModel(GetCurrentModel()->GetAssetFilepath());
    }
}

void MainScene::LoadModel(const Path &modelPath)
{
    p_currentModel = Assets::Load<Model>(modelPath);
    GetView3DScene()->OnModelChanged(GetCurrentModel());
    GetUvScene()->OnModelChanged(GetCurrentModel());
    GetTexturesScene()->OnModelChanged(GetCurrentModel());
}

void MainScene::SetSceneMode(MainScene::SceneMode sceneMode)
{
    if (sceneMode != GetSceneMode())
    {
        m_renderMode = sceneMode;
        GetControlPanel()->SetSceneModeOnComboBox(GetSceneMode());
    }
}

Model *MainScene::GetCurrentModel() const
{
    return p_currentModel.Get();
}

GameObject *MainScene::GetCurrentModelGameObject() const
{
    return GetView3DScene()->GetModelGameObject();
}

View3DScene *MainScene::GetView3DScene() const
{
    return p_view3DScene;
}

UvScene *MainScene::GetUvScene() const
{
    return p_uvScene;
}

TexturesScene *MainScene::GetTexturesScene() const
{
    return p_texturesScene;
}

SceneImage *MainScene::GetSceneImage() const
{
    return p_sceneImage;
}

ControlPanel *MainScene::GetControlPanel() const
{
    return p_controlPanel;
}

MainScene::SceneMode MainScene::GetSceneMode() const
{
    return m_renderMode;
}

MainScene *MainScene::GetInstance()
{
    return s_mainScene;
}

Scene *MainScene::GetCurrentRenderScene() const
{
    switch (GetSceneMode())
    {
        case SceneMode::VIEW3D: return GetView3DScene(); break;
        case SceneMode::UV: return GetUvScene(); break;
        case SceneMode::TEXTURES: return GetTexturesScene(); break;
    }
    return nullptr;
}
