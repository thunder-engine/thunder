#ifndef FBXCONVERTER_H
#define FBXCONVERTER_H

#include "converters/converter.h"

#include "resources/mesh.h"

class MeshSerial;

class Actor;

namespace ofbx {
    class IScene;
    class Object;
    class Mesh;
}

class FbxImportSettings : public IConverterSettings {
    Q_OBJECT

    Q_PROPERTY(bool Use_Custom_Scale READ useScale WRITE setUseScale DESIGNABLE true USER true)
    Q_PROPERTY(float Custom_Scale READ customScale WRITE setCustomScale DESIGNABLE true USER true)
    Q_PROPERTY(bool Import_Color READ colors WRITE setColors DESIGNABLE true USER true)
    Q_PROPERTY(bool Import_Normals READ normals WRITE setNormals DESIGNABLE true USER true)
    Q_PROPERTY(bool Import_Animation READ animation WRITE setAnimation DESIGNABLE true USER true)

public:
    FbxImportSettings();

    bool colors() const;
    void setColors(bool value);

    bool normals() const;
    void setNormals(bool value);

    bool animation() const;
    void setAnimation(bool value);

    bool useScale() const;
    void setUseScale(bool value);

    float customScale() const;
    void setCustomScale(float value);

protected:
    bool m_UseScale;
    float m_Scale;

    bool m_Colors;
    bool m_Normals;

    bool m_Animation;

};

class FBXConverter : public IConverter {
public:
    FBXConverter();

    QStringList suffixes() const { return {"fbx"}; }
    uint32_t contentType() const { return ContentPrefab; }
    uint32_t type() const { return MetaType::type<Actor *>(); }
    uint8_t convertFile(IConverterSettings *);

    IConverterSettings *createSettings() const;

protected:
    Actor *importObject(const ofbx::Object *element, FbxImportSettings *settings, QStringList &list);

    MeshSerial *importMesh(const ofbx::Mesh *m, float scale);
};

#endif // FBXCONVERTER_H
