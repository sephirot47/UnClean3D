#ifndef UIEFFECTLAYERS_H
#define UIEFFECTLAYERS_H

#include "Bang/Array.h"
#include "Bang/Bang.h"
#include "Bang/GameObject.h"
#include "Bang/IEventsUIList.h"
#include "BangEditor/BangEditor.h"

namespace Bang
{
class UIList;
class UIToolButton;
class UIScrollPanel;
}
using namespace Bang;

class EffectLayer;
class UIEffectLayerRow;

class UIEffectLayers : public GameObject, public EventListener<IEventsUIList>
{
public:
    UIEffectLayers();
    virtual ~UIEffectLayers() override;

    // GameObject
    void Update() override;

    UIEffectLayerRow *CreateNewEffectLayerRow(EffectLayer *newEffectLayer);
    void RemoveEffectLayer(uint effectLayerIdx);

    void SetSelection(uint idx);
    void SetSelection(UIEffectLayerRow *effectLayerRow);
    void ClearLayersSelection();
    bool IsAllLayersVisibleButtonOn() const;
    void Duplicate(UIEffectLayerRow *effectLayerRow);
    void Clear();

    UIList *GetList() const;
    uint GetSelectedEffectLayerRowIndex() const;
    UIEffectLayerRow *GetSelectedEffectLayerRow() const;
    const Array<UIEffectLayerRow *> &GetUIEffectLayerRows() const;

private:
    Array<UIEffectLayerRow *> p_effectLayerRows;

    UIToolButton *p_allLayersVisibleButton = nullptr;
    UIScrollPanel *p_scrollPanel = nullptr;
    GameObject *p_listContainer = nullptr;
    UIList *p_uiList = nullptr;

    // IEventsUIList
    virtual void OnItemMoved(GameObject *item,
                             int oldIndex,
                             int newIndex) override;
};

#endif  // UIEFFECTLAYERS_H
