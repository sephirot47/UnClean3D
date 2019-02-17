#ifndef PULLPUSH_H
#define PULLPUSH_H

#include "Bang/AssetHandle.h"
#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

namespace Bang
{
class Texture2D;
class Framebuffer;
class ShaderProgram;
};

using namespace Bang;

class PullPush
{
public:
    PullPush();
    virtual ~PullPush();

    void ReloadShaders();
    void PullPushTexture(Texture2D *texture);

private:
    Framebuffer *m_framebuffer = nullptr;
    AH<ShaderProgram> m_pullPushSP;
    AH<Texture2D> m_informationMaskTexture;
    AH<Texture2D> m_texture0;
    AH<Texture2D> m_texture1;
};

#endif  // PULLPUSH_H
