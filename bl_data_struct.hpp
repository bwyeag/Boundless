/*
 * BOUNDLESS Engine File
 * Create: 2023/12/3/10:56
 * Data Struct C++ Header
 *
 */
#ifndef _BOUNDLESS_DATA_STRUCT_HPP_FILE_
#define _BOUNDLESS_DATA_STRUCT_HPP_FILE_
#include "glad/glad.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <list>
#include <vector>

#include "bl_log.hpp"

namespace Boundless {
template <typename DT, size_t chunk_size = 128>
class object_pool {
   private:
    union block {
        block* ptr;
        DT data;
    };
    struct obj_chunk {
        obj_chunk* next;
        block blocks[chunk_size];
    };
    obj_chunk* chunk_head;
    block* free_head;
    explicit object_pool(size_t init_chunks) {
        if (init_chunks == 0) {
            chunk_head = nullptr;
            free_head = nullptr;
            return;
        }

        obj_chunk* p = new obj_chunk();
        for (size_t i = 0; i < chunk_size - 1; i--) {
            p->blocks[i].ptr = &p->blocks[i + 1];
        }
        p->blocks[chunk_size - 1].ptr = nullptr;
        free_head = &p->blocks[0];
        p->next = nullptr;
        chunk_head = p;

        for (size_t i = 1; i < init_chunks; i++) {
            obj_chunk* p = new obj_chunk();
            for (size_t i = 0; i < chunk_size - 1; i--) {
                p->blocks[i].ptr = &p->blocks[i + 1];
            }
            p->blocks[chunk_size - 1].ptr = free_head;
            free_head = &p->blocks[0];
            p->next = chunk_head;
            chunk_head = p;
        }
    }
    object_pool& object_pool(object_pool&&) = delete;
    object_pool& object_pool(const object_pool&) = delete;
    object_pool& operator=(object_pool&&) = delete;
    object_pool& operator=(const object_pool&) = delete;
    ~object_pool() {
        obj_chunk *p = chunk_head, q;
        while (!p) {
            q = p->next;
            free(p);
            p = q;
        }
    }
}
}  // namespace Boundless
#endif  //!_BOUNDLESS_DATA_STRUCT_HPP_FILE_