#ifndef UIEFFECTLAYERS_H
#define UIEFFECTLAYERS_H

#include "Bang/Array.h"
#include "Bang/Bang.h"
#include "Bang/GameObject.h"
#include "BangEditor/BangEditor.h"

namespace Bang
{
class UIScrollPanel;
}
using namespace Bang;

class UIEffectLayerRow;

class UIEffectLayers : public GameObject
{
public:
    UIEffectLayers();
    virtual ~UIEffectLayers();

    UIEffectLayerRow *CreateRow();

private:
    Array<UIEffectLayerRow *> p_effectLayerRows;

    UIScrollPanel *p_scrollPanel = nullptr;
    GameObject *p_scrollPanelContainer = nullptr;
};

#endif  // UIEFFECTLAYERS_H
