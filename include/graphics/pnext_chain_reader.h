//
// Created by Saman on 17.04.23.
//

#ifndef REALTIME_CELL_COLLAPSE_PNEXT_CHAIN_READER_H
#define REALTIME_CELL_COLLAPSE_PNEXT_CHAIN_READER_H

#include <vulkan/vulkan.h>

// TODO Test
namespace PointerChains {
    VkBaseOutStructure *find(void *start, const VkStructureType &type) {
        if(start == nullptr) return nullptr;
        auto current = reinterpret_cast<VkBaseOutStructure *>(start);
        while (true) {
            if (current->sType == type) {
                return current;
            } else if (current->pNext == nullptr) {
                return nullptr;
            } else {
                current = reinterpret_cast<VkBaseOutStructure *>(current->pNext);
            }
        }
    }
}


#endif //REALTIME_CELL_COLLAPSE_PNEXT_CHAIN_READER_H
