#ifndef FBXCONVERTER_H
#define FBXCONVERTER_H

#include <fbxsdk.h>

#include "baseconvertersettings.h"

class Mesh;
class IFile;

class FBXConverter : public IConverter {
public:
    FBXConverter                ();

    ~FBXConverter               ();

    string                      format                  () const;
    IConverter::ContentTypes    type                    () const;
    uint8_t                     convertFile             (IConverterSettings *);

protected:
    void                        importFBX               (const string &src, Mesh &mesh);
    void                        importDynamic           (KFbxNode *lRootNode, Mesh &mesh);
    void                        importAFAnimation       (KFbxScene *lScene);

    void                        saveAnimation           (const string &dst);

};

#endif // FBXCONVERTER_H
