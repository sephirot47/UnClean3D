#ifndef TEXTURECONTAINERROW_H
#define TEXTURECONTAINERROW_H

#include "Bang/Bang.h"
#include "Bang/GameObject.h"
#include "BangEditor/BangEditor.h"

using namespace Bang;

class EffectLayer;
class EffectLayerMask;
class TextureContainer;

class TextureContainerRow : public GameObject
{
public:
    TextureContainerRow(EffectLayer *effectLayer = nullptr);
    virtual ~TextureContainerRow();

    TextureContainer *CreateAndAddTextureContainer(const String &name = "");
    TextureContainer *CreateAndAddEffectColorTextureContainer(
        EffectLayer *effectLayer,
        const String &name = "");
    TextureContainer *CreateAndAddMergedMaskTextureContainer(
        EffectLayer *effectLayer,
        const String &name = "");
    TextureContainer *CreateAndAddMaskTextureContainer(
        EffectLayerMask *effectLayerMask,
        const String &name = "");
    void RemoveTextureContainer(TextureContainer *textureContainer);

    bool CanBeDestroyed() const;
    EffectLayer *GetEffectLayer() const;
    TextureContainer *GetEffectColorTextureContainer() const;
    TextureContainer *GetMergedMaskEffectTextureContainer() const;
    const Map<EffectLayerMask *, TextureContainer *>
        &GetMaskTextureContainersMap() const;

private:
    EffectLayer *p_effectLayer = nullptr;
    TextureContainer *p_effectColorTextureContainer = nullptr;
    TextureContainer *p_mergedMaskEffectTextureContainer = nullptr;
    Map<EffectLayerMask *, TextureContainer *> p_maskTextureContainers;
};

#endif  // TEXTURECONTAINERROW_H
