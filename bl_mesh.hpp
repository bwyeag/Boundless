#ifndef _BOUNDLESS_MESH_HPP_FILE_
#define _BOUNDLESS_MESH_HPP_FILE_
#include "glad/glad.h"
#include <vector>
#include <omp.h>
#include "bl_log.hpp"
#include "bl_resource_load.hpp"

namespace Boundless::Resource
{
    typedef float(*BinaryFunctionFloat)(float,float);
    struct mesh_pack
    {
        size_t vertex_size;
        float* vertex_ptr;
        size_t index_size;
        void* index_ptr;
        GLenum index_type;
        GLsizeiptr index_count;
        uint32_t restart_index;
    };

    class MeshGeneraters
    {
    public:
        /// @brief 生成正方形网格函数
        /// @param x x方向上生成的格点数
        /// @param y y方向上生成的格点数
        /// @param F 二元函数，指定z值
        /// @param ptrs 返回值包装
        static void GenMeshRectangle(uint32_t x,uint32_t y,BinaryFunctionFloat F,mesh_pack* ptrs);
    };
} // namespace Boundless::Resource
namespace Boundless::Render
{
    using Resource::IndexStatus;
    struct Mesh
    {
        GLuint vertex_array;
        GLuint vertex_buffer, index_buffer;
        GLuint restart_index;
        IndexStatus index_status;
        std::vector<GLuint> buffers;

        void Initialize(size_t buffer_count,IndexStatus ist);
        void Initialize(const Resource::MeshLoader& mesh);
        void Destroy();
    };
} // namespace Boundless::Render

#endif //!_BOUNDLESS_MESH_HPP_FILE_