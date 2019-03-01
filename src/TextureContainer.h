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
class UILayoutElement;
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
    void SetLabel(const String &label);
    void SetCanBeFocused(bool canBeFocused);
    void SetSelected(bool selected);

    UILabel *GetLabel() const;
    UIImageRenderer *GetBorder() const;
    UIFocusable *GetFocusable() const;
    UIImageRenderer *GetImageRenderer() const;
    UILayoutElement *GetImageLayoutElement() const;

private:
    bool m_canBeFocused = false;
    UIImageRenderer *p_border = nullptr;
    UIImageRenderer *p_imageRenderer = nullptr;
    UILayoutElement *p_imageLE = nullptr;
    UIFocusable *p_focusable = nullptr;
    UILabel *p_label = nullptr;
};

#endif  // TEXTURECONTAINER_H
