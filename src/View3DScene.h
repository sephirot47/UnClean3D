#ifndef VIEW3DSCENE_H
#define VIEW3DSCENE_H

#include "Bang/AssetHandle.h"
#include "Bang/Bang.h"
#include "Bang/FPSChrono.h"
#include "Bang/Map.h"
#include "Bang/Scene.h"
#include "Bang/ShaderProgram.h"
#include "Bang/Vector2.h"
#include "Bang/Vector3.h"
#include "BangEditor/BangEditor.h"

namespace Bang
{
class Camera;
class Model;
class MeshRenderer;
};
using namespace Bang;

class EffectLayer;
class ControlPanel;

class View3DScene : public Scene
{
public:
    View3DScene();
    virtual ~View3DScene() override;

    // Scene
    void Update() override;
    void Render(RenderPass rp, bool renderChildren) override;

    void ReloadShaders();
    void OnModelChanged(Model *newModel);

    void CreateNewEffectLayer();
    void InvalidateEffectLayersTextures();

    Camera *GetCamera() const;
    GameObject *GetModelGameObject() const;
    Model *GetCurrentModel() const;

private:
    FPSChrono m_fpsChrono;
    Model *p_currentModel = nullptr;
    GameObject *p_modelContainer = nullptr;
    Map<MeshRenderer *, Array<EffectLayer *>> m_meshRendererToEffectLayers;

    bool m_effectLayersValid = false;
    AH<ShaderProgram> m_view3DShaderProgram;

    Camera *p_cam = nullptr;
    bool m_orbiting = false;
    float m_currentCameraZoom = 0.0f;
    Vector3 m_cameraOrbitPoint = Vector3::Zero();
    Vector2 m_currentCameraRotAngles = Vector2::Zero();

    void ResetCamera();
    ControlPanel *GetControlPanel() const;
};

#endif  // VIEW3DSCENE_H
