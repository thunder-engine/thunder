#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION

#include "wrappermt.h"
#include "resources/texture.h"

MTL::Device *WrapperMt::s_device = nullptr;

MTL::CommandQueue *WrapperMt::s_queue = nullptr;

MTL::Device *WrapperMt::device() {
    if(s_device == nullptr) {
        s_device = MTL::CreateSystemDefaultDevice();

        if(s_device->supportsFeatureSet(MTL::FeatureSet_iOS_GPUFamily4_v1) || s_device->supportsFeatureSet(MTL::FeatureSet_iOS_GPUFamily3_v1)) {
            Texture::setMaxTextureSize(16384); // Common for A9 chips and newer
        } else if(s_device->supportsFeatureSet(MTL::FeatureSet_iOS_GPUFamily2_v2) || s_device->supportsFeatureSet(MTL::FeatureSet_iOS_GPUFamily1_v2)) {
            Texture::setMaxTextureSize(8192);
        }

        s_queue = s_device->newCommandQueue();
    }

    return s_device;
}

MTL::CommandQueue *WrapperMt::queue() {
    return s_queue;
}
