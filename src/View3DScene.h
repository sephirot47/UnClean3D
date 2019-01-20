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

#include "ControlPanel.h"
#include "EffectLayer.h"

namespace Bang
{
class Camera;
class Model;
class Framebuffer;
class MeshRenderer;
class LineRenderer;
};
using namespace Bang;

class EffectLayer;
class EffectLayerCompositer;
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

    EffectLayer *CreateNewEffectLayer();
    void RemoveEffectLayer(uint effectLayerIdx);
    void UpdateParameters(const EffectLayerParameters &params);
    void ApplyControlPanelSettingsToModel();
    void ApplyCompositeTexturesToModel();
    void SetViewUniforms();
    void PaintMaskBrush();
    void RestoreOriginalAlbedoTexturesToModel();

    Camera *GetCamera() const;
    GameObject *GetModelGameObject() const;
    const Vector3 &GetModelOriginalLocalScale() const;
    Array<EffectLayer *> GetAllEffectLayers() const;
    Array<EffectLayer *> GetSelectedEffectLayers() const;
    Model *GetCurrentModel() const;

private:
    struct MeshRendererInfo
    {
        AH<Texture2D> originalAlbedoTexture;
        AH<Texture2D> originalNormalTexture;
        AH<Texture2D> originalRoughnessTexture;
        AH<Texture2D> originalMetalnessTexture;
        Array<EffectLayer *> effectLayers;
    };

    bool m_validTextures = false;
    Time m_lastTimeTexturesGenerated = Time::Zero();
    LineRenderer *p_maskBrushRend = nullptr;

    FPSChrono m_fpsChrono;
    Model *p_currentModel = nullptr;
    GameObject *p_modelContainer = nullptr;
    Vector3 m_originalModelLocalScale = Vector3::One();
    Map<MeshRenderer *, MeshRendererInfo> m_meshRendererToInfo;

    AH<ShaderProgram> m_view3DShaderProgram;
    EffectLayerCompositer *m_effectLayerCompositer = nullptr;

    Camera *p_cam = nullptr;
    bool m_orbiting = false;
    float m_currentCameraZoom = 0.0f;
    Vector3 m_cameraOffset = Vector3::Zero();
    Vector3 m_cameraOrbitPoint = Vector3::Zero();
    Vector2 m_currentCameraRotAngles = Vector2::Zero();

    void ResetCamera();
    ControlPanel *GetControlPanel() const;
};

#endif  // VIEW3DSCENE_H
