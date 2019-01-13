#ifndef UIEFFECTLAYERROW_H
#define UIEFFECTLAYERROW_H

#include "Bang/Bang.h"
#include "Bang/GameObject.h"
#include "BangEditor/BangEditor.h"

namespace Bang
{
class UILabel;
class UIFocusable;
class UIToolButton;
class UIImageRenderer;
}

using namespace Bang;

class UIEffectLayers;

class UIEffectLayerRow : public GameObject
{
public:
    UIEffectLayerRow(UIEffectLayers *uiEffectLayer);
    virtual ~UIEffectLayerRow();

    // GameObject
    void Update() override;

    String GetName() const;
    bool IsSelected() const;
    bool GetIsLayerVisible() const;

private:
    UIToolButton *p_visibleButton = nullptr;
    UIFocusable *p_focusable = nullptr;
    UIEffectLayers *p_uiEffectLayers = nullptr;
    UIImageRenderer *p_bg = nullptr;
    UILabel *p_layerNameLabel = nullptr;
};

#endif  // UIEFFECTLAYERROW_H
