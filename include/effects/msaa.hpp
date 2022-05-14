#include "texture.hpp"

namespace msaa {
Texture<vec3> msaa_filter(const Texture<std::array<vec3, 4>> &src) {
    Texture<vec3> res(src.width, src.height, vec3(0, 0, 0));
    for (size_t i = 0; i < src.width * src.height; i++) {
        for (size_t j = 0; j < 4; j++) {
            res[i] += src[i][j];
        }
        res[i] /= 4;
    }
    return res;
}
}  // namespace msaa