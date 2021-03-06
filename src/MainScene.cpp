#include "MainScene.h"

#include "Bang/Assets.h"
#include "Bang/Camera.h"
#include "Bang/GBuffer.h"
#include "Bang/GEngine.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Input.h"
#include "Bang/Paths.h"
#include "Bang/RectTransform.h"
#include "Bang/UICanvas.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"

#include "Clipboard.h"
#include "ControlPanel.h"
#include "SceneImage.h"
#include "TexturesScene.h"
#include "View3DScene.h"

using namespace Bang;

MainScene *MainScene::s_mainScene = nullptr;

MainScene::MainScene()
{
    MainScene::s_mainScene = this;

    GameObjectFactory::CreateUISceneInto(this);
    UIHorizontalLayout *hl = AddComponent<UIHorizontalLayout>();
    p_view3DScene = new View3DScene();
    p_texturesScene = new TexturesScene();

    p_sceneImage = new SceneImage();
    GetSceneImage()->SetParent(this);

    p_controlPanel = new ControlPanel();
    GetControlPanel()->SetParent(this);

    m_clipboard = new Clipboard();
}

MainScene::~MainScene()
{
    delete m_clipboard;
}

void MainScene::Start()
{
    if (!IsStarted())
    {
        Path projAssets = Paths::GetProjectAssetsDir();
        Path modelPath;
        // modelPath = projAssets.Append("Models/cube.dae");
        // modelPath = projAssets.Append("Models/MyTable/MyTable.dae");
        // modelPath = projAssets.Append("Models/MyTable/untitled.dae");
        modelPath = projAssets.Append("Models/Character/Character.dae");
        // modelPath = projAssets.Append("Models/Dresser/Dresser.dae");
        // modelPath = projAssets.Append("Models/BrickWall/brickWall.ply");
        // modelPath = projAssets.Append("Models/bunny.ply");
        // modelPath = projAssets.Append("Models/armadillo.ply");
        // modelPath = projAssets.Append("Models/streetLamp.ply");
        // modelPath = projAssets.Append("Models/dwarf.ply");
        // modelPath = projAssets.Append("Models/dragon.ply");
        // modelPath = projAssets.Append("Models/Column/Column.dae");
        // modelPath = projAssets.Append("Models/Bench/Bench.dae");
        GetControlPanel()->OpenModel(modelPath);
    }

    Scene::Start();
}

void MainScene::Update()
{
    Scene::Update();

    if (Input::GetKeyDown(Key::R))
    {
        // ResetModel();
    }
    else if (Input::GetKeyDown(Key::F5))
    {
        SetSceneMode(MainScene::SceneMode::VIEW3D);
    }
    else if (Input::GetKeyDown(Key::F6))
    {
        SetSceneMode(MainScene::SceneMode::TEXTURES);
    }

    // Always update view3D scene
    if (GetSceneMode() != MainScene::SceneMode::VIEW3D)
    {
        GetView3DScene()->Update();
    }
}

void MainScene::Render(RenderPass renderPass, bool renderChildren)
{
    if (renderPass == RenderPass::CANVAS)
    {
        GL::Push(GL::Pushable::VIEWPORT);
        Input::Context prevContext = Input::GetContext();

        AARecti renderRect(
            GetSceneImage()->GetRectTransform()->GetViewportAARect());
        Vector2i renderSize(renderRect.GetSize());
        renderSize = Vector2i::Max(renderSize, Vector2i::One());

        Input::Context sceneContext;
        sceneContext.rect = renderRect;
        Input::SetContext(sceneContext);

        Scene *scene = GetCurrentRenderScene();
        scene->GetCamera()->SetRenderSize(renderSize);
        GL::SetViewport(renderRect);

        scene->Update();
        GEngine::GetInstance()->Render(scene, scene->GetCamera());

        GetSceneImage()->SetImageTexture(
            scene->GetCamera()->GetGBuffer()->GetDrawColorTexture());

        Input::SetContext(prevContext);
        GL::Pop(GL::Pushable::VIEWPORT);
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

bool MainScene::IsMouseOverView3DScene() const
{
    return GetSceneImage()->GetFocusable()->IsMouseOver();
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

Clipboard *MainScene::GetClipboard() const
{
    return m_clipboard;
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
        case SceneMode::TEXTURES: return GetTexturesScene(); break;
    }
    return nullptr;
}
