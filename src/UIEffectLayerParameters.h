#ifndef UIEFFECTLAYERPARAMETERS_H
#define UIEFFECTLAYERPARAMETERS_H

#include "Bang/Bang.h"
#include "Bang/GameObject.h"
#include "Bang/IEventsValueChanged.h"
#include "BangEditor/BangEditor.h"

namespace Bang
{
class UIComboBox;
class UIToolButton;
class UISlider;
};

namespace BangEditor
{
class UIInputColor;
};

class EffectLayer;

using namespace Bang;
using namespace BangEditor;

class UIEffectLayerParameters : public GameObject,
                                public EventListener<IEventsValueChanged>
{
public:
    UIEffectLayerParameters();
    virtual ~UIEffectLayerParameters() override;

    void SetEffectLayer(EffectLayer *effectLayer);

    EffectLayer *GetEffectLayer() const;

private:
    EffectLayer *p_effectLayer = nullptr;

    UIInputColor *p_colorInput = nullptr;
    UIToolButton *p_colorVisibleButton = nullptr;
    UIComboBox *p_colorBlendModeInput = nullptr;

    UISlider *p_heightInput = nullptr;
    UIToolButton *p_heightVisibleButton = nullptr;
    UIComboBox *p_heightBlendModeInput = nullptr;

    UISlider *p_roughnessInput = nullptr;
    UIToolButton *p_roughnessVisibleButton = nullptr;
    UIComboBox *p_roughnessBlendModeInput = nullptr;

    UISlider *p_metalnessInput = nullptr;
    UIToolButton *p_metalnessVisibleButton = nullptr;
    UIComboBox *p_metalnessBlendModeInput = nullptr;

    // IEventsValueChanged
    virtual void OnValueChanged(EventEmitter<IEventsValueChanged> *ee) override;
};

#endif  // UIEFFECTLAYERPARAMETERS_H
