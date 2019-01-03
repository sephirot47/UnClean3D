#ifndef EDITSCENE_H
#define EDITSCENE_H

#include "Bang/AssetHandle.h"
#include "Bang/Bang.h"
#include "Bang/Camera.h"
#include "Bang/Model.h"
#include "Bang/Scene.h"
#include "Bang/Vector2.h"
#include "Bang/Vector3.h"
#include "BangEditor/BangEditor.h"

using namespace Bang;

class EditScene : public Scene
{
public:
    EditScene();
    virtual ~EditScene();

    void Update() override;

    void OpenModel(const Path &modelPath);
    void RenderScene(const Vector2i &renderSize);

    Camera *GetCamera() const;

private:
    AH<Model> p_currentModel;
    GameObject *p_modelContainer = nullptr;

    Camera *p_cam = nullptr;
    float m_currentCameraZoom = 0.0f;
    Vector3 m_cameraOrbitPoint = Vector3::Zero();
    Vector2 m_currentCameraRotAngles = Vector2::Zero();

    void ResetCamera();
};

#endif  // EDITSCENE_H
