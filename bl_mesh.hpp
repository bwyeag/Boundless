#ifndef _BOUNDLESS_MESH_HPP_FILE_
#define _BOUNDLESS_MESH_HPP_FILE_
#include "glad/glad.h"
#include <omp.h>

namespace Boundless::Resource
{
    typedef float(*BinaryFunctionFloat)(float,float);
    struct mesh_pack
    {
        size_t vertex_size;
        size_t index_size;
        float* vertex_ptr;
        void* index_ptr;
        GLenum index_type;
        uint32_t restart_index;
        GLsizeiptr index_count;
    };
    void GenerateMeshRectangle(uint32_t x,uint32_t y,BinaryFunctionFloat,mesh_pack*);


} // namespace Boundless::Resource

#endif //!_BOUNDLESS_MESH_HPP_FILE_