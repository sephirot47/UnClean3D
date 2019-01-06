#include "Bang/Application.h"

#include "Bang/BehaviourContainer.h"
#include "Bang/BehaviourManager.h"
#include "Bang/Camera.h"
#include "Bang/Debug.h"
#include "Bang/Extensions.h"
#include "Bang/GameObject.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/Library.h"
#include "Bang/Material.h"
#include "Bang/Mesh.h"
#include "Bang/MeshFactory.h"
#include "Bang/MeshRenderer.h"
#include "Bang/MetaFilesManager.h"
#include "Bang/Model.h"
#include "Bang/Paths.h"
#include "Bang/PointLight.h"
#include "Bang/PostProcessEffect.h"
#include "Bang/RectTransform.h"
#include "Bang/Scene.h"
#include "Bang/SceneManager.h"
#include "Bang/Shader.h"
#include "Bang/TextureFactory.h"
#include "Bang/Transform.h"
#include "Bang/UITextRenderer.h"
#include "Bang/Window.h"
#include "Bang/WindowManager.h"
#include "BangEditor/EditorApplication.h"
#include "BangEditor/UIInputFile.h"

#include "MainScene.h"

using namespace Bang;
using namespace BangEditor;

int main(int, char **)
{
    EditorApplication app;
    {
        const Path editorPath =
            Paths::GetExecutableDir().GetDirectory().Append("BangEditor");
        app.Init(editorPath.Append("Bang"), editorPath);
    }

    Window *mainWindow = WindowManager::CreateWindow<Window>();
    Window::SetActive(mainWindow);
    mainWindow->SetTitle("UnClean3D");
    mainWindow->Maximize();

    ProjectManager::GetInstance()->OpenProject(
        Paths::GetExecutableDir().GetDirectory().Append("UnClean3D.bproject"));

    Path AssetsPath =
        Paths::GetExecutablePath().GetDirectory().GetDirectory().Append(
            "Assets");
    MetaFilesManager::CreateMissingMetaFiles(AssetsPath);

    MainScene *mainScene = new MainScene();
    SceneManager::LoadSceneInstantly(mainScene, false);

    return app.MainLoop();
}
