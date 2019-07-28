#ifndef AAMBIENTOCCLUSIONGL_H
#define AAMBIENTOCCLUSIONGL_H

#include "postprocessor.h"

#include <amath.h>

class AmbientOcclusion : public PostProcessor {
public:
    AmbientOcclusion ();

    ~AmbientOcclusion ();

protected:
    Vector3 *m_pSamplesKernel;

};

#endif // AAMBIENTOCCLUSIONGL_H
