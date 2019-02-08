#ifndef GLSLARRAYOFARRAYS_H
#define GLSLARRAYOFARRAYS_H

#include "Bang/AssetHandle.h"
#include "Bang/Bang.h"
#include "Bang/GL.h"
#include "Bang/Texture2D.h"
#include "BangEditor/BangEditor.h"

using namespace Bang;

class GLSLArrayOfArrays
{
public:
    GLSLArrayOfArrays();
    virtual ~GLSLArrayOfArrays();

    void SetFormat(GL::ColorFormat colorFormat);
    void Fill(const Array<Array<Vector4>> &array);

    Texture2D *GetArrayTexture() const;
    void Bind(const String &arrayName, ShaderProgram *sp);

private:
    const int TexSize = 2048;
    AH<Texture2D> m_arrayTexture;
};

#endif  // GLSLARRAYOFARRAYS_H
