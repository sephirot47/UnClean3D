#ifndef TEXTURECONTAINER_H
#define TEXTURECONTAINER_H

#include "Bang/Bang.h"
#include "Bang/GameObject.h"
#include "Bang/String.h"
#include "BangEditor/BangEditor.h"

namespace Bang
{
class UILabel;
class UIFocusable;
class UIImageRenderer;
}

using namespace Bang;

class TextureContainer : public GameObject
{
public:
    TextureContainer(const String &label = "Label");
    virtual ~TextureContainer() override;

    // GameObject
    void Update() override;

    UILabel *GetLabel() const;
    UIFocusable *GetFocusable() const;
    UIImageRenderer *GetImageRenderer() const;

private:
    UIImageRenderer *p_imageRenderer = nullptr;
    UIFocusable *p_focusable = nullptr;
    UILabel *p_label = nullptr;
};

#endif  // TEXTURECONTAINER_H
