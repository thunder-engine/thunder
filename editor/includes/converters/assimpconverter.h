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
typedef std::map<TString, Actor *> ActorsMap;
typedef std::unordered_map<uint32_t, Mesh *> MeshMap;

class AssimpImportSettings : public AssetConverterSettings {
    A_OBJECT(AssimpImportSettings, AssetConverterSettings, Editor)

    A_PROPERTIES(
        A_PROPERTY(bool, Use_Custom_Scale, AssimpImportSettings::useScale, AssimpImportSettings::setUseScale),
        A_PROPERTY(float, Custom_Scale, AssimpImportSettings::customScale, AssimpImportSettings::setCustomScale),
        A_PROPERTY(bool, Import_Color, AssimpImportSettings::colors, AssimpImportSettings::setColors),
        A_PROPERTY(bool, Import_Normals, AssimpImportSettings::normals, AssimpImportSettings::setNormals),
        A_PROPERTY(bool, Import_Animation, AssimpImportSettings::animation, AssimpImportSettings::setAnimation),
        A_PROPERTYEX(Compression, Compress_Animation, AssimpImportSettings::filter, AssimpImportSettings::setFilter, "enum=Compression"),
        A_PROPERTY(float, Position_Error, AssimpImportSettings::positionError, AssimpImportSettings::setPositionError),
        A_PROPERTY(float, Rotation_Error, AssimpImportSettings::rotationError, AssimpImportSettings::setRotationError),
        A_PROPERTY(float, Scale_Error, AssimpImportSettings::scaleError, AssimpImportSettings::setScaleError)
    )
    A_ENUMS(
        A_ENUM(Compression,
               A_VALUE(Off),
               A_VALUE(Keyframe_Reduction))
    )

public:
    enum Compression {
        Off = 0,
        Keyframe_Reduction
    };

    AssimpImportSettings();

    bool colors() const;
    void setColors(bool value);

    bool normals() const;
    void setNormals(bool value);

    bool animation() const;
    void setAnimation(bool value);

    int filter() const;
    void setFilter(int value);

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

    BonesList m_bones;

    MeshMap m_meshes;

    ActorsMap m_actors;

    Actor *m_rootActor;
    Actor *m_rootBone;

    bool m_flip;

private:
    StringList typeNames() const override;

protected:
    bool m_useScale;
    float m_scale;

    bool m_colors;
    bool m_normals;

    bool m_animation;
    int m_filter;

    float m_positionError;
    float m_rotationError;
    float m_scaleError;

};

class AssimpConverter : public AssetConverter {
public:
    AssimpConverter();

    void init() override;

    StringList suffixes() const override { return {"fbx", "obj", "gltf", "glb"}; }
    ReturnCode convertFile(AssetConverterSettings *) override;

    AssetConverterSettings *createSettings() override;

    Actor *createActor(const AssetConverterSettings *settings, const TString &guid) const override;

    Actor *importObject(const aiScene *scene, const aiNode *element, Actor *parent, AssimpImportSettings *fbxSettings);

    static Mesh *importMesh(const aiScene *scene, const aiNode *element, Actor *actor, AssimpImportSettings *fbxSettings);

    static void importAnimation(const aiScene *scene, AssimpImportSettings *fbxSettings);

    static void importPose(AssimpImportSettings *fbxSettings);

};

#endif // ASSIMPCONVERTER_H
