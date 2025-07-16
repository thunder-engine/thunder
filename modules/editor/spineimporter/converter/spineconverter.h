#ifndef SPINECONVERTER_H
#define SPINECONVERTER_H

#include <editor/assetconverter.h>

class Mesh;
class Sprite;
class Transform;
class SpriteRender;

namespace {
    const char *gX("x");
    const char *gY("y");
    const char *gWidth("width");
    const char *gHeight("height");
    const char *gScaleX("scaleX");
    const char *gScaleY("scaleY");
    const char *gRotation("rotation");
    const char *gAnimations("animations");
    const char *gAttachment("attachment");
    const char *gAttachments("attachments");
    const char *gBounds("bounds");
    const char *gBones("bones");
    const char *gBone("bone");
    const char *gColor("color");
    const char *gName("name");
    const char *gType("type");
    const char *gSlots("slots");
    const char *gSlot("slot");
    const char *gSkins("skins");
    const char *gParent("parent");
    const char *gPath("path");
    const char *gTriangles("triangles");
    const char *gVertices("vertices");
    const char *gOffsets("offsets");
    const char *gOffset("offset");
    const char *gRotate("rotate");
    const char *gTranslate("translate");
    const char *gPosition("position");
    const char *gScale("scale");
    const char *gTime("time");
    const char *gAngle("angle");

    const char *gSpriteRender("SpriteRender");
    const char *gTransform("Transform");
};

struct Item {
    Vector4 bounds;
    float rotate;
};

struct Slot {
    String color;
    String bone;
    String item;
    uint32_t layer;

    SpriteRender *render = nullptr;
};

class SpineConverterSettings : public AssetConverterSettings {
public:
    SpineConverterSettings();

    float customScale() const;
    void setCustomScale(float value);

private:
    QString defaultIconPath(const QString &) const override;

public:
    std::map<String, Actor *> m_boneStructure;

    std::map<String, Slot> m_slots;

    std::map<String, Item> m_atlasItems;

    Vector2 m_atlasSize;

    Actor *m_root;

private:
    float m_scale;

};

class SpineConverter : public AssetConverter {
public:
    static String pathTo(Object *root, Object *dst);

    static Vector4 toColor(const String &color);

private:
    QStringList suffixes() const override { return {"spine"}; }
    ReturnCode convertFile(AssetConverterSettings *settings) override;
    AssetConverterSettings *createSettings() override;
    Actor *createActor(const AssetConverterSettings *settings, const QString &guid) const override;

    Actor *importBones(const VariantList &bones, SpineConverterSettings *settings);
    void importSlots(const VariantList &list, SpineConverterSettings *settings);
    void importSkins(const VariantList &list, SpineConverterSettings *settings);

    void importAnimations(const VariantMap &animations, SpineConverterSettings *settings);

    void importAtlas(Sprite *sprite, SpineConverterSettings *settings);

    void importRegion(const VariantMap &fields, const String &itemName, Transform *transform, Mesh *mesh, SpineConverterSettings *settings);
    void importMesh(const VariantMap &fields, const String &itemName, Mesh *mesh, SpineConverterSettings *settings);

    void stabilizeUUID(Object *object);

};

#endif //SPINECONVERTER_H
