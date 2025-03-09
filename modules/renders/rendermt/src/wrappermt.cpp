#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION

#include "wrappermt.h"

MTL::Device *WrapperMt::s_device = nullptr;

MTL::Device *WrapperMt::device() {
    if(s_device == nullptr)
    {
        s_device = MTL::CreateSystemDefaultDevice();
    }

    return s_device;
}
