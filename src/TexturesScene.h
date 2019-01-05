#ifndef TEXTURESSCENE_H
#define TEXTURESSCENE_H

#include "Bang/Bang.h"
#include "Bang/Model.h"
#include "Bang/Scene.h"
#include "Bang/UIImageRenderer.h"
#include "BangEditor/BangEditor.h"

using namespace Bang;

class TexturesScene : public Scene
{
public:
    TexturesScene();
    virtual ~TexturesScene();

    // Scene
    void Update() override;

    void OnModelChanged(Model *model);

private:
    UIImageRenderer *p_albedoImgRend = nullptr;
};

#endif  // TEXTURESSCENE_H
