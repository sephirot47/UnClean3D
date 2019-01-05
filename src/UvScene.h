#ifndef UVSCENE_H
#define UVSCENE_H

#include "Bang/Bang.h"
#include "Bang/Model.h"
#include "Bang/Scene.h"
#include "BangEditor/BangEditor.h"

using namespace Bang;

class UvScene : public Scene
{
public:
    UvScene();
    virtual ~UvScene();

    // Scene
    void Update() override;

    void OnModelChanged(Model *newModel);

private:
    Camera *p_cam = nullptr;
    GameObject *p_modelContainer = nullptr;

    GameObject *GetModelGameObject() const;
};

#endif  // UVSCENE_H
