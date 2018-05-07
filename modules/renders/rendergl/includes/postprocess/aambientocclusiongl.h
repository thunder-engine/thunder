#ifndef AAMBIENTOCCLUSIONGL_H
#define AAMBIENTOCCLUSIONGL_H

#include "postprocess/apostprocessor.h"

class AAmbientOcclusionGL : public APostProcessor {
public:
    AAmbientOcclusionGL         ();

    ~AAmbientOcclusionGL        ();

protected:

    Vector3                    *m_pSamplesKernel;

};

#endif // AAMBIENTOCCLUSIONGL_H
