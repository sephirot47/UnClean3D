#ifndef UIEFFECTLAYERMASKROW_H
#define UIEFFECTLAYERMASKROW_H

#include "Bang/Bang.h"
#include "Bang/GameObject.h"
#include "Bang/IEventsValueChanged.h"
#include "BangEditor/BangEditor.h"

namespace Bang
{
class UIComboBox;
class UIImageRenderer;
};

using namespace Bang;

class UIEffectLayerMaskRow : public GameObject,
                             public EventListener<IEventsValueChanged>
{
public:
    UIEffectLayerMaskRow();
    virtual ~UIEffectLayerMaskRow() override;

private:
    UIComboBox *p_maskTypeInput = nullptr;

    // IEventsValueChanged
    virtual void OnValueChanged(EventEmitter<IEventsValueChanged> *ee) override;
};

#endif  // UIEFFECTLAYERMASKROW_H
