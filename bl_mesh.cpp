#include "bl_mesh.hpp"

namespace Boundless::Resource
{
    void MeshGeneraters::GenMeshRectangle(uint32_t x, uint32_t y, BinaryFunctionFloat F, mesh_pack *ptrs)
    {
        ptrs->vertex_size = sizeof(float) * x * y * 3;
        ptrs->vertex_ptr = (float *)malloc(ptrs->vertex_size);
        ptrs->restart_index = x * y + 1;
        {
            const float dx = 1.0f / (x - 1), dy = 1.0f / (y - 1);
#pragma omp parallel shared(dx, dy, ptrs, x, y)
            {
                float px, py, *ptr;
#pragma omp for
                for (size_t j = 0; j < y; j++)
                {
                    ptr = &ptrs->vertex_ptr[3 * x * j];
                    py = dy * j;
                    for (size_t i = 0; i < x; i++)
                    {
                        *ptr = dx * i;
                        *(ptr + 1) = py;
                        *(ptr + 2) = F(px, py);
                        ptr += 3;
                    }
                }
            }
        }
        ptrs->index_count = (y - 1) * (2 * x + 1);
        if (x * y >= 0xFFFFU)
        {
            ptrs->index_type = GL_UNSIGNED_INT;
            ptrs->index_size = sizeof(GLuint) * (y - 1) * (2 * x + 1);
            ptrs->index_ptr = (GLuint *)malloc(ptrs->index_size);
            const size_t dline = 2 * x + 1;
            const GLuint ri = x * y + 1;
#pragma omp parallel shared(dline, ptrs, x, y)
            {
                GLuint *ptr, c;
#pragma omp for
                for (size_t j = 0; j < y - 1; j++)
                {
                    ptr = &((GLuint *)ptrs->index_ptr)[dline * j];
                    c = x * j;
                    for (size_t i = 0; i < x; i++)
                    {
                        *ptr = c + x, *(ptr + 1) = c;
                        ptr += 2;
                        c++;
                    }
                    *ptr = ri;
                }
            }
        }
        else
        {
            ptrs->index_type = GL_UNSIGNED_SHORT;
            ptrs->index_size = sizeof(GLushort) * (y - 1) * (2 * x + 1);
            ptrs->index_ptr = (GLushort *)malloc(ptrs->index_size);
            const size_t dline = 2 * x + 1;
            const GLushort ri = x * y + 1;
#pragma omp parallel shared(dline, ptrs, x, y)
            {
                GLushort *ptr, c;
#pragma omp for
                for (size_t j = 0; j < y - 1; j++)
                {
                    ptr = &((GLushort *)ptrs->index_ptr)[dline * j];
                    c = x * j;
                    for (size_t i = 0; i < x; i++)
                    {
                        *ptr = c + x, *(ptr + 1) = c;
                        ptr += 2;
                        c++;
                    }
                    *ptr = ri;
                }
            }
        }
    }
} // namespace Boundless::Resource

namespace Boundless::Render
{
    void Mesh::Initialize(size_t buffer_count,IndexStatus ist)
    {
        glCreateVertexArrays(1, &vertex_array);
        index_status = ist;
        if (ist != IndexStatus::NONE)
        {
            glCreateBuffers(2, &vertex_buffer);
        }
        else
        {
            glCreateBuffers(1, &vertex_buffer);
        }
        buffers.resize(buffer_count);
        glCreateBuffers(buffer_count, &buffers[0]);
    }
    void Mesh::Initialize(const Resource::MeshLoader &mesh)
    {
        glCreateVertexArrays(1, &vertex_array);
        index_status = mesh.GetIndexStatus();
        auto &bufs = mesh.GetBuffers();
        vertex_buffer = bufs[0];
        if (index_status == IndexStatus::NONE)
        {
            restart_index = 0xFFFFFFFFU;
            buffers.resize(bufs.size() - 1);
            for (size_t i = 1, j = 0; i < bufs.size(); i++, j++)
            {
                buffers[j] = bufs[i];
            }
        }
        else
        {
            index_buffer = bufs[1];
            buffers.resize(bufs.size() - 2);
            for (size_t i = 2, j = 0; i < bufs.size(); i++, j++)
            {
                buffers[j] = bufs[i];
            }
            if (index_status == IndexStatus::INDEX_WITH_RESTARTINDEX)
                restart_index = mesh.GetRestartIndex();
            else if (index_status == IndexStatus::ONLY_INDEX)
                restart_index = 0xFFFFFFFFU;
        }
    }
    void Mesh::Destroy()
    {
        glDeleteBuffers(1, &vertex_buffer);
        if (index_status != IndexStatus::NONE)
        {
            glDeleteBuffers(1, &index_buffer);
        }
        glDeleteBuffers(buffers.size(), &buffers[0]);
    }
} // namespace Boundless::Render