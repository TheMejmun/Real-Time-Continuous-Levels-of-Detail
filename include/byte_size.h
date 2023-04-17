//
// Created by Saman on 17.04.23.
//

#ifndef REALTIME_CELL_COLLAPSE_BYTE_SIZE_H
#define REALTIME_CELL_COLLAPSE_BYTE_SIZE_H


#include <cstdint>

namespace ByteSize {
    inline float KB(const uint32_t &bytes) {
        return static_cast<float>(bytes) / 1024.0f;
    }

    inline float MB(const uint32_t &bytes) {
        return static_cast<float>(bytes) / (1024.0f * 1024.0f);
    }

    inline float GB(const uint32_t &bytes) {
        return static_cast<float>(bytes) / (1024.0f * 1024.0f * 1024.0f);
    }
}

#endif //REALTIME_CELL_COLLAPSE_BYTE_SIZE_H
