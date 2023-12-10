#include "data_struct.hpp"

namespace Boundless
{
    void MakeSurfaceRectangle(uint32_t x, uint32_t y, Math_F2 F, ptr_pack *ptrs)
    {
        ptrs->vertex_size = sizeof(float) * x * y;
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

}