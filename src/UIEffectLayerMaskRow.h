#ifndef UIEFFECTLAYERMASKROW_H
#define UIEFFECTLAYERMASKROW_H

#include "Bang/Bang.h"
#include "Bang/GameObject.h"
#include "Bang/IEventsValueChanged.h"
#include "BangEditor/BangEditor.h"

namespace Bang
{
class UILabel;
class UIButton;
class UIComboBox;
class UIToolButton;
class UIImageRenderer;
};

namespace BangEditor
{
class UIContextMenu;
};

using namespace Bang;
using namespace BangEditor;

class UIEffectLayers;
class EffectLayerMask;
class UIEffectLayerRow;
class UIEffectLayerMaskRow : public GameObject,
                             public EventListener<IEventsValueChanged>
{
public:
    UIEffectLayerMaskRow(UIEffectLayerRow *uiEffectLayerRow,
                         EffectLayerMask *effectLayerMask);
    virtual ~UIEffectLayerMaskRow() override;

    void UpdateFromEffectLayerMask();

    void SetUIEffectLayerRow(UIEffectLayerRow *uiEffectLayerRow);
    void SetEffectLayerMask(EffectLayerMask *effectLayerMask);

    EffectLayerMask *GetEffectLayerMask() const;
    UIEffectLayerRow *GetEffectLayerRow() const;

private:
    UIToolButton *p_visibleButton = nullptr;
    UIEffectLayers *p_uiEffectLayers = nullptr;
    UIEffectLayerRow *p_uiEffectLayerRow = nullptr;
    EffectLayerMask *p_effectLayerMask = nullptr;

    UIComboBox *p_blendModeInput = nullptr;
    UIContextMenu *p_contextMenu = nullptr;
    UILabel *p_nameLabel = nullptr;
    UIComboBox *p_maskTypeInput = nullptr;
    UIButton *p_removeButton = nullptr;

    // IEventsValueChanged
    virtual void OnValueChanged(EventEmitter<IEventsValueChanged> *ee) override;
};

#endif  // UIEFFECTLAYERMASKROW_H
