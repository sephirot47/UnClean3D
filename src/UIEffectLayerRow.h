#ifndef UIEFFECTLAYERROW_H
#define UIEFFECTLAYERROW_H

#include "Bang/Bang.h"
#include "Bang/GameObject.h"
#include "Bang/IEventsValueChanged.h"
#include "BangEditor/BangEditor.h"

namespace Bang
{
class UILabel;
class UIComboBox;
class UIFocusable;
class UIToolButton;
class UIImageRenderer;
}

using namespace Bang;

class EffectLayer;
class UIEffectLayers;

class UIEffectLayerRow : public GameObject,
                         public EventListener<IEventsValueChanged>
{
public:
    UIEffectLayerRow(UIEffectLayers *uiEffectLayer, EffectLayer *effectLayer);
    virtual ~UIEffectLayerRow() override;

    // GameObject
    void Update() override;

    String GetName() const;
    bool IsSelected() const;
    bool GetIsLayerVisible() const;
    UIToolButton *GetIsLayerVisibleButton() const;

private:
    UIEffectLayers *p_uiEffectLayers = nullptr;
    EffectLayer *p_effectLayer = nullptr;

    UIToolButton *p_visibleButton = nullptr;
    UIFocusable *p_focusable = nullptr;
    UIImageRenderer *p_bg = nullptr;
    UIComboBox *p_effectLayerTypeInput = nullptr;
    UILabel *p_layerNameLabel = nullptr;

    // IEventsValueChanged
    void OnValueChanged(EventEmitter<IEventsValueChanged> *ee) override;
};

#endif  // UIEFFECTLAYERROW_H
