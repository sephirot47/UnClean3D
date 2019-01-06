#ifndef DIRTER_H
#define DIRTER_H

#include "Bang/Bang.h"
#include "Bang/MeshRenderer.h"
#include "Bang/Texture2D.h"
#include "BangEditor/BangEditor.h"

using namespace Bang;

class Dirter
{
public:
    Dirter() = delete;
    ~Dirter() = delete;

    static void AddDirt(GameObject *modelGameObject);
    static void AddDirtToTexture(MeshRenderer *mr, Texture2D *texture);
};

#endif  // DIRTER_H
