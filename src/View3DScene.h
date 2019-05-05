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
#include "GLSLRayCaster.h"

namespace Bang
{
class Camera;
class Model;
class Framebuffer;
class MeshRenderer;
class LineRenderer;
class DirectionalLight;
};
using namespace Bang;

class EffectLayer;
class EffectLayerMask;
class EffectLayerCompositer;
class ControlPanel;

class View3DScene : public Scene
{
public:
    enum class Environment
    {
        OPEN_SEA,
        YOKOHAMA_NIGHT,
        PARK,
        HOTEL
    };

    View3DScene();
    virtual ~View3DScene() override;

    // Scene
    void Update() override;
    void Render(RenderPass rp, bool renderChildren) override;

    void ReloadShaders();
    void OnModelChanged(Model *newModel);

    EffectLayer *CreateNewEffectLayer();
    void RemoveEffectLayer(uint effectLayerIdx);

    void CompositeTextures();
    void ApplyControlPanelSettingsToModel();
    void ApplyCompositeTexturesToModel();
    void RestoreOriginalAlbedoTexturesToModel();
    void MoveEffectLayer(EffectLayer *effectLayer, uint newIndex);
    void SetEnvironment(Environment environment);
    void RegenerateAllEffectLayers();
    void InvalidateAll();

    Camera *GetCamera() const;
    GLSLRayCaster *GetGLSLRayCaster() const;
    GameObject *GetModelGameObject() const;
    View3DScene::Environment GetEnvironment() const;
    const Vector3 &GetModelOriginalLocalScale() const;
    Array<EffectLayer *> GetAllEffectLayers() const;
    Array<EffectLayer *> GetSelectedEffectLayers() const;
    Array<EffectLayerMask *> GetSelectedEffectLayerMasks() const;
    EffectLayerCompositer *GetEffectLayerCompositer() const;
    Texture2D *GetEnvironmentSnapshot(Environment environment) const;
    Model *GetCurrentModel() const;
    Mesh *GetTextureMesh() const;
    EffectLayerCompositer *GetCompositer() const;

    static View3DScene *GetInstance();

private:
    struct MeshRendererInfo
    {
        AH<Texture2D> originalAlbedoTexture;
        AH<Texture2D> originalNormalTexture;
        AH<Texture2D> originalRoughnessTexture;
        AH<Texture2D> originalMetalnessTexture;
        Array<EffectLayer *> effectLayers;
    };

    Time m_lastTimeTexturesValidated = Time::Zero();
    GLSLRayCaster *m_glslRayCaster = nullptr;
    MeshRenderer *p_maskBrushPreviewRenderer = nullptr;
    AH<ShaderProgram> m_brushPreviewSP;

    FPSChrono m_fpsChrono;
    AH<Mesh> m_textureMesh;
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
    Environment m_environment = Environment::OPEN_SEA;

    float m_currentCamAngleY = 0.0f;
    float m_currentCamAngleX = 0.0f;
    DirectionalLight *p_dirLight = nullptr;

    AH<Texture2D> m_openSeaSS;
    AH<TextureCubeMap> m_parkCM;
    AH<Texture2D> m_parkSS;
    AH<TextureCubeMap> m_hotelCM;
    AH<Texture2D> m_hotelSS;
    AH<TextureCubeMap> m_yokohamaCM;
    AH<Texture2D> m_yokohamaSS;

    void ResetCamera();
    ControlPanel *GetControlPanel() const;
};

#endif  // VIEW3DSCENE_H
