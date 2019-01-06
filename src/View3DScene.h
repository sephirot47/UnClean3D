#ifndef VIEW3DSCENE_H
#define VIEW3DSCENE_H

#include "Bang/AssetHandle.h"
#include "Bang/Bang.h"
#include "Bang/Camera.h"
#include "Bang/FPSChrono.h"
#include "Bang/Model.h"
#include "Bang/Scene.h"
#include "Bang/Vector2.h"
#include "Bang/Vector3.h"
#include "BangEditor/BangEditor.h"

using namespace Bang;

class View3DScene : public Scene
{
public:
    View3DScene();
    virtual ~View3DScene();

    // Scene
    void Update() override;

    void OnModelChanged(Model *newModel);

    Camera *GetCamera() const;
    GameObject *GetModelGameObject() const;
    Model *GetCurrentModel() const;

private:
    FPSChrono m_fpsChrono;
    Model *p_currentModel = nullptr;
    GameObject *p_modelContainer = nullptr;

    Camera *p_cam = nullptr;
    bool m_orbiting = false;
    float m_currentCameraZoom = 0.0f;
    Vector3 m_cameraOrbitPoint = Vector3::Zero();
    Vector2 m_currentCameraRotAngles = Vector2::Zero();

    void ResetCamera();
};

#endif  // VIEW3DSCENE_H
