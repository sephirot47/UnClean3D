#ifndef PULLPUSH_H
#define PULLPUSH_H

#include "Bang/AssetHandle.h"
#include "Bang/Bang.h"
#include "Bang/Map.h"
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
    AH<ShaderProgram> m_pullPushPrepareTextureSP;
    Map<int, AH<Texture2D>> m_pullTextures;
    Map<int, AH<Texture2D>> m_pushTextures;

    Texture2D *GetPullTexture(int size);
    Texture2D *GetPushTexture(int size);
    Texture2D *GetPullPushTexture(int size, bool pull);
};

#endif  // PULLPUSH_H
