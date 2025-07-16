#ifndef ASSIMPCONVERTER_H
#define ASSIMPCONVERTER_H

#include <editor/assetconverter.h>

class Actor;
class Mesh;

class aiScene;
class aiNode;
class aiMesh;
class aiBone;
class aiAnimation;

typedef std::list<const aiBone *> BonesList;
typedef std::map<String, Actor *> ActorsMap;
typedef std::unordered_map<uint32_t, Mesh *> MeshMap;

class AssimpImportSettings : public AssetConverterSettings {
    Q_OBJECT

    Q_PROPERTY(bool Use_Custom_Scale READ useScale WRITE setUseScale DESIGNABLE true USER true)
    Q_PROPERTY(float Custom_Scale READ customScale WRITE setCustomScale DESIGNABLE true USER true)
    Q_PROPERTY(bool Import_Color READ colors WRITE setColors DESIGNABLE true USER true)
    Q_PROPERTY(bool Import_Normals READ normals WRITE setNormals DESIGNABLE true USER true)

    Q_PROPERTY(bool Import_Animation READ animation WRITE setAnimation DESIGNABLE true USER true)
    Q_PROPERTY(Compression Compress_Animation READ filter WRITE setFilter DESIGNABLE true USER true)
    Q_PROPERTY(float Position_Error READ positionError WRITE setPositionError DESIGNABLE true USER true)
    Q_PROPERTY(float Rotation_Error READ rotationError WRITE setRotationError DESIGNABLE true USER true)
    Q_PROPERTY(float Scale_Error READ scaleError WRITE setScaleError DESIGNABLE true USER true)

public:

    enum Compression {
        Off = 0,
        Keyframe_Reduction
    };
    Q_ENUM(Compression)

    AssimpImportSettings();

    bool colors() const;
    void setColors(bool value);

    bool normals() const;
    void setNormals(bool value);

    bool animation() const;
    void setAnimation(bool value);

    Compression filter() const;
    void setFilter(Compression value);

    bool useScale() const;
    void setUseScale(bool value);

    float customScale() const;
    void setCustomScale(float value);

    float positionError() const;
    void setPositionError(float value);

    float rotationError() const;
    void setRotationError(float value);

    float scaleError() const;
    void setScaleError(float value);

    Object::ObjectList m_renders;

    QStringList m_resources;

    BonesList m_bones;

    MeshMap m_meshes;

    ActorsMap m_actors;

    Actor *m_rootActor;
    Actor *m_rootBone;

    bool m_flip;

private:
    QStringList typeNames() const override;

    QString defaultIconPath(const QString &) const override;

protected:
    bool m_useScale;
    float m_scale;

    bool m_colors;
    bool m_normals;

    bool m_animation;
    Compression m_filter;

    float m_positionError;
    float m_rotationError;
    float m_scaleError;

};

class AssimpConverter : public AssetConverter {
public:
    AssimpConverter();

    QStringList suffixes() const override { return {"fbx", "obj", "gltf", "glb"}; }
    ReturnCode convertFile(AssetConverterSettings *) override;

    AssetConverterSettings *createSettings() override;

    Actor *createActor(const AssetConverterSettings *settings, const QString &guid) const override;

    Actor *importObject(const aiScene *scene, const aiNode *element, Actor *parent, AssimpImportSettings *fbxSettings);

    static Mesh *importMesh(const aiScene *scene, const aiNode *element, Actor *actor, AssimpImportSettings *fbxSettings);

    static void importAnimation(const aiScene *scene, AssimpImportSettings *fbxSettings);

    static void importPose(AssimpImportSettings *fbxSettings);

};

#endif // ASSIMPCONVERTER_H
