#include "data_struct.hpp"

namespace Boundless
{
    void MakeSurfaceRectangle(uint32_t x, uint32_t y, Math_F2 F, ptr_pack *ptrs)
    {
        ptrs->ver = sizeof(float) * x * y;
        ptrs->vertex_position = (float *)malloc(ptrs->ver);
        ptrs->restart_index = x * y + 1;
        {
            float dx = 1.0f / (x - 1), dy = 1.0f / (y - 1), px = 0.0f, py = 0.0f;
            size_t i = 0, j = 0;
            float *p = ptrs->vertex_position;
            for (; j < y; j++, py += dy)
            {
                for (; i < x; i++, px += dx)
                {
                    *p = px;
                    *(p + 1) = py;
                    *(p + 2) = F(px, py);
                    p += 3;
                }
                i=0;
            }
        }
        ptrs->count = (y-1) * (2*x+1);
        if (x * y >= 0xFFFFU)
        {
            ptrs->type = INDEX_UINT32;
            ptrs->ver = sizeof(uint32_t) * (y-1) * (2*x+1);
            ptrs->index = malloc(ptrs->ver);
            uint32_t* p = (uint32_t*)ptrs->index;
            uint32_t c = 0, ri = x*y+1;
            size_t i = 0,j = 0;
            for (; j < y-1; j++)
            {
                for (; i < x; i++)
                {
                    *p = c+x,*(p+1)=c;
                    p+=2;c++;
                }
                *p=ri;p++;
                i = 0;
            }
        }
        else
        {
            ptrs->type = INDEX_UINT16;
            ptrs->ver = sizeof(uint16_t) * (y-1) * (2*x+1);
            ptrs->index = malloc(ptrs->ver);
            uint16_t* p = (uint16_t*)ptrs->index;
            uint16_t c = 0, ri = x*y+1;
            size_t i = 0,j = 0;
            for (; j < y-1; j++)
            {
                for (; i < x; i++)
                {
                    *p = c+x,*(p+1)=c;
                    p+=2;c++;
                }
                *p=ri;p++;
                i = 0;
            }
            
        }
    }

}