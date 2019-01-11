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
    Dirter(MeshRenderer *mr);
    virtual ~Dirter();

    void CreateDirtTexture();

    Texture2D *GetDirtTexture() const;

private:
    AH<Texture2D> m_dirtTexture;
    MeshRenderer *p_meshRenderer = nullptr;
};

#endif  // DIRTER_H
