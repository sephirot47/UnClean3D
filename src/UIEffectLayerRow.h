#ifndef UIEFFECTLAYERROW_H
#define UIEFFECTLAYERROW_H

#include "Bang/Bang.h"
#include "Bang/GameObject.h"
#include "Bang/IEventsUIList.h"
#include "Bang/IEventsValueChanged.h"
#include "BangEditor/BangEditor.h"

namespace Bang
{
class UIList;
class UILabel;
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

    String GetName() const;
    bool IsSelected() const;
    bool GetIsLayerVisible() const;
    UIFocusable *GetFocusable() const;
    EffectLayer *GetEffectLayer() const;
    UIToolButton *GetIsLayerVisibleButton() const;

private:
    UIImageRenderer *p_whiteBackground = nullptr;
    UIEffectLayers *p_uiEffectLayers = nullptr;
    EffectLayer *p_effectLayer = nullptr;
    UIContextMenu *p_contextMenu = nullptr;

    UIList *p_innerList = nullptr;
    Array<GameObject *> p_maskRows;

    UIToolButton *p_visibleButton = nullptr;
    UIFocusable *p_focusable = nullptr;
    UIComboBox *p_effectLayerTypeInput = nullptr;
    UILabel *p_layerNameLabel = nullptr;

    // IEventsValueChanged
    void OnValueChanged(EventEmitter<IEventsValueChanged> *ee) override;
};

#endif  // UIEFFECTLAYERROW_H
