#ifndef UIEFFECTLAYERMASKROW_H
#define UIEFFECTLAYERMASKROW_H

#include "Bang/Bang.h"
#include "Bang/GameObject.h"
#include "Bang/IEventsValueChanged.h"
#include "BangEditor/BangEditor.h"

namespace Bang
{
class UIButton;
class UIComboBox;
class UIImageRenderer;
};

using namespace Bang;

class UIEffectLayerRow;
class UIEffectLayerMaskRow : public GameObject,
                             public EventListener<IEventsValueChanged>
{
public:
    UIEffectLayerMaskRow();
    virtual ~UIEffectLayerMaskRow() override;

    void SetUIEffectLayerRow(UIEffectLayerRow *uiEffectLayerRow);

private:
    UIEffectLayerRow *p_effectLayerRow = nullptr;

    UIComboBox *p_maskTypeInput = nullptr;
    UIButton *p_removeButton = nullptr;

    // IEventsValueChanged
    virtual void OnValueChanged(EventEmitter<IEventsValueChanged> *ee) override;
};

#endif  // UIEFFECTLAYERMASKROW_H
