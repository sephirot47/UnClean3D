#ifndef UIEFFECTLAYERS_H
#define UIEFFECTLAYERS_H

#include "Bang/Array.h"
#include "Bang/Bang.h"
#include "Bang/GameObject.h"
#include "BangEditor/BangEditor.h"

namespace Bang
{
class UIToolButton;
class UIScrollPanel;
}
using namespace Bang;

class EffectLayer;
class UIEffectLayerRow;

class UIEffectLayers : public GameObject
{
public:
    UIEffectLayers();
    virtual ~UIEffectLayers();

    // GameObject
    void Update() override;

    UIEffectLayerRow *CreateNewEffectLayerRow(EffectLayer *newEffectLayer);
    void RemoveEffectLayer(uint effectLayerIdx);

    void SetSelection(uint idx);
    void SetSelection(UIEffectLayerRow *effectLayerRow);
    void ClearLayersSelection();
    bool IsAllLayersVisibleButtonOn() const;

    uint GetSelectedEffectLayerRowIndex() const;
    UIEffectLayerRow *GetSelectedEffectLayerRow() const;
    const Array<UIEffectLayerRow *> &GetUIEffectLayerRows() const;

private:
    uint m_selectedEffectLayerRowIndex = SCAST<uint>(-1);
    Array<UIEffectLayerRow *> p_effectLayerRows;

    UIToolButton *p_allLayersVisibleButton = nullptr;
    UIScrollPanel *p_scrollPanel = nullptr;
    GameObject *p_listContainer = nullptr;
};

#endif  // UIEFFECTLAYERS_H
