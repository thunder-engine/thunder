#ifndef RENDERTARGETMT_H
#define RENDERTARGETMT_H

#include <resources/rendertarget.h>

#include "wrappermt.h"

struct Global;

class RenderTargetMt : public RenderTarget {
    A_OBJECT_OVERRIDE(RenderTargetMt, RenderTarget, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    RenderTargetMt();

    MTL::RenderPassDescriptor *nativeHandle();
    void setNativeHandle(MTL::RenderPassDescriptor *descriptor);

    void setLevel(uint32_t level);

    MTL::Buffer *globalBuffer(size_t currentFrame);
    void updateGlobalMemory(size_t currentFrame, const Global &global);

    bool isNative() const;

private:
    std::vector<MTL::Buffer *> m_global;

    MTL::RenderPassDescriptor *m_descriptor;

    uint32_t m_currentLevel;

    bool m_native;

};

#endif // RENDERTARGETMT_H
