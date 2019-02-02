#ifndef UIEFFECTLAYERROW_H
#define UIEFFECTLAYERROW_H

#include "Bang/Bang.h"
#include "Bang/GameObject.h"
#include "Bang/IEventsUIList.h"
#include "Bang/IEventsValueChanged.h"
#include "Bang/UIList.h"
#include "BangEditor/BangEditor.h"

namespace Bang
{
class UILabel;
class UIButton;
class UIComboBox;
class UIFocusable;
class UIToolButton;
class UIImageRenderer;
};

namespace BangEditor
{
class UIContextMenu;
};

using namespace Bang;
using namespace BangEditor;

class EffectLayer;
class UIEffectLayerMaskRow;
class UIEffectLayers;

class UIEffectLayerRow : public GameObject,
                         public EventListener<IEventsValueChanged>,
                         public EventListener<IEventsUIList>
{
public:
    UIEffectLayerRow(UIEffectLayers *uiEffectLayer, EffectLayer *effectLayer);
    virtual ~UIEffectLayerRow() override;

    // GameObject
    void Update() override;
    void UpdateFromEffectLayer();

    UIEffectLayerMaskRow *AddNewMaskRow();
    void Duplicate(UIEffectLayerMaskRow *maskRow);
    void RemoveMaskRow(UIEffectLayerMaskRow *maskRow);

    String GetName() const;
    bool IsSelected() const;
    bool GetIsLayerVisible() const;
    UIList *GetMaskRowsList() const;
    UIFocusable *GetFocusable() const;
    EffectLayer *GetEffectLayer() const;
    UIEffectLayers *GetUIEffectLayers() const;
    UIToolButton *GetIsLayerVisibleButton() const;

private:
    UIEffectLayers *p_uiEffectLayers = nullptr;
    EffectLayer *p_effectLayer = nullptr;
    UIContextMenu *p_contextMenu = nullptr;

    UIList *p_maskRowsList = nullptr;
    Array<UIEffectLayerMaskRow *> p_maskRows;

    GameObject *p_addNewMaskRow = nullptr;
    UIToolButton *p_visibleButton = nullptr;
    UIButton *p_addNewMaskButton = nullptr;
    UIFocusable *p_focusable = nullptr;
    UILabel *p_layerNameLabel = nullptr;

    void OnMasksListCallback(GameObject *item, UIList::Action action);

    // IEventsValueChanged
    void OnValueChanged(EventEmitter<IEventsValueChanged> *ee) override;
};

#endif  // UIEFFECTLAYERROW_H
