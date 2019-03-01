#ifndef BRUSHINSPECTORWIDGET_H
#define BRUSHINSPECTORWIDGET_H

#include "Bang/Array.h"
#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"
#include "BangEditor/SerializableInspectorWidget.h"

using namespace Bang;
using namespace BangEditor;

class TextureContainer;
class BrushInspectorWidget : public SerializableInspectorWidget
{
public:
    BrushInspectorWidget();
    virtual ~BrushInspectorWidget() override;

    // InspectorWidget
    virtual void InitInnerWidgets() override;
    virtual void UpdateFromReference() override;

    const Array<TextureContainer *> &GetTextureContainers() const;

private:
    Array<TextureContainer *> p_textureContainers;
    GameObject *p_brushesImagesContainer = nullptr;
};

#endif  // BRUSHINSPECTORWIDGET_H
