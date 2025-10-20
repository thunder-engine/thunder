#include <encoder/basisu_comp.h>

#include <resources/texture.h>

#include "textureconverter.h"

bool TextureConverter::compress(Texture *texture) {
    bool result = basisu::basisu_encoder_init();

    if(result) {
        basisu::job_pool jpool(1);

        basisu::basis_compressor_params params;
        params.m_multithreading = true;
        params.m_pJob_pool = &jpool;

        switch(texture->compress()) {
            case Texture::ETC1: {
                params.m_uastc = false;
                params.m_etc1s_quality_level = 128; // 1-255;
                params.m_etc1s_max_endpoint_clusters = 512;
                params.m_etc1s_max_selector_clusters = 512;
            } break;
            case Texture::ASTC: {
                params.m_uastc = true;
                params.m_pack_uastc_ldr_4x4_flags = basisu::cPackUASTCLevelDefault;
                params.m_rdo_uastc_ldr_4x4_quality_scalar = 1.0f;
            } break;
            default: return false;
        }

        uint32_t w = texture->width();
        uint32_t h = texture->height();

        uint32_t channels = 4;
        if(texture->format() == Texture::RGB8) {
            channels = 3;
        }

        result = false;
        for(uint32_t s = 0; s < texture->sides(); s++) {
            Texture::Surface &surface = texture->surface(0);

            params.m_source_images.push_back(basisu::image(surface[0].data(), w, h, channels));

            basisu::basis_compressor comp;
            if(comp.init(params)) {
                if(comp.process() == basisu::basis_compressor::cECSuccess) {
                    const basisu::uint8_vec &data = comp.get_output_basis_file();

                    ByteArray output;
                    output.assign(data.begin(), data.end());

                    result = true;
                }
            }
        }

        basisu::basisu_encoder_deinit();
    }

    return result;
}
