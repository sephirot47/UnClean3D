#ifndef MAINSCENE_H
#define MAINSCENE_H

#include "Bang/AssetHandle.h"
#include "Bang/Bang.h"
#include "Bang/Model.h"
#include "Bang/Scene.h"
#include "BangEditor/BangEditor.h"

class UvScene;
class View3DScene;
class TexturesScene;
class SceneImage;
class ControlPanel;
class Clipboard;

using namespace Bang;

class MainScene : public Scene
{
public:
    enum class SceneMode
    {
        VIEW3D,
        TEXTURES
    };

    MainScene();
    virtual ~MainScene() override;

    // Scene
    void Start() override;
    void Update() override;
    void Render(RenderPass renderPass, bool renderChildren) override;

    void ResetModel();
    void LoadModel(const Path &modelPath);
    void SetSceneMode(MainScene::SceneMode sceneMode);

    bool IsMouseOverView3DScene() const;
    Model *GetCurrentModel() const;
    GameObject *GetCurrentModelGameObject() const;
    View3DScene *GetView3DScene() const;
    UvScene *GetUvScene() const;
    TexturesScene *GetTexturesScene() const;
    SceneImage *GetSceneImage() const;
    ControlPanel *GetControlPanel() const;
    SceneMode GetSceneMode() const;
    Clipboard *GetClipboard() const;

    static MainScene *GetInstance();

private:
    static MainScene *s_mainScene;

    AH<Model> p_currentModel;

    SceneMode m_renderMode = SceneMode::VIEW3D;
    UvScene *p_uvScene = nullptr;
    View3DScene *p_view3DScene = nullptr;
    TexturesScene *p_texturesScene = nullptr;
    SceneImage *p_sceneImage = nullptr;
    ControlPanel *p_controlPanel = nullptr;
    Clipboard *m_clipboard = nullptr;

    Scene *GetCurrentRenderScene() const;
};

#endif  // MAINSCENE_H
