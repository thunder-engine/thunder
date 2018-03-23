#ifndef FBXCONVERTER_H
#define FBXCONVERTER_H

#include <fbxsdk.h>

#include "baseconvertersettings.h"

#include "resources/mesh.h"

class IFile;

class MeshSerial : public Mesh {
public:

    VariantMap                  saveUserData                () const;

protected:
    friend class FBXConverter;

};

class FBXConverter : public IConverter {
public:
    FBXConverter                ();

    ~FBXConverter               ();

    string                      format                  () const;
    IConverter::ContentTypes    type                    () const;
    uint8_t                     convertFile             (IConverterSettings *);

protected:
    void                        importFBX               (const string &src, Mesh &mesh);
    void                        importDynamic           (FbxNode *lRootNode, Mesh &mesh);
    void                        importAFAnimation       (FbxScene *lScene);

    void                        saveAnimation           (const string &dst);

};

#endif // FBXCONVERTER_H
