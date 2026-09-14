/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
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

uint32_t WrapperMt::framesInFlight() {
    return 2;
}
