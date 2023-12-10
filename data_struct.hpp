/*
 * BOUNDLESS Engine File
 * Create: 2023/12/3/10:56
 * Data Struct C++ Header
 *
 */
#ifndef _BOUNDLESS_DATA_STRUCT_HPP_FILE_
#define _BOUNDLESS_DATA_STRUCT_HPP_FILE_
#include <omp.h>
#include "glad/glad.h"

#include <cstdint>
#include <cstddef>

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <vector>
#include <list>
#include <algorithm>

#define ERROR(type, info) std::cerr << "[ERROR][" << type << "]file:" << __FILE__ << ";line:" << __LINE__ << "|info:" << info << std::endl;
#define ERRORINFO(info) std::cerr << "[ERROR]other:" << info << std::endl;
#define WARNING(type, info) std::cerr << "[WARNING][" << type << "]file:" << __FILE__ << ";line:" << __LINE__ << "|info:" << info << std::endl;
#define WARNINGINFO(info) std::cerr << "[WARNING]other:" << info << std::endl;
#define INFO(type, info) std::cout << "[INFO][" << type << "]file:" << __FILE__ << ";line:" << __LINE__ << "|info:" << info << std::endl;
#define INFODATA(info) std::cout << "[INFO]other:" << info << std::endl;

namespace Boundless
{
    typedef float(*Math_F2)(float,float);
    struct ptr_pack
    {
        size_t vertex_size;
        size_t index_size;
        float* vertex_ptr;
        void* index_ptr;
        GLenum index_type;
        uint32_t restart_index;
        GLsizeiptr index_count;
    };
    void MakeSurfaceRectangle(uint32_t x,uint32_t y,Math_F2,ptr_pack*);

    template<typename Type, size_t blockSize>
    class ObjectPool
    {
    private:
        struct obj_block
        {
            union block
            {
                Type* ptr;
                Type obj;
            } data[blockSize];
        };
        std::vector<obj_block*> blockPointers;
        Type* nullobjRoot;
        Type* nullobjBack;
        size_t poolSize = 0;
    public:
        ObjectPool(size_t init_blocks)
        {
            init_blocks = std::max(init_blocks,(size_t)1);
            obj_block* ptr;
            blockPointers.resize(init_blocks);
            size_t i,j;
            for (i = 0; i < init_blocks; i++)
            {
                ptr = new obj_block();
                if (ptr == nullptr)
                {
                    ERROR("MEMORY","内存耗尽!")
                    exit(-1);
                }
                blockPointers[i] = ptr;
            }
            std::sort(blockPointers.begin(),blockPointers.end(),std::greater<obj_block*>());

            Type** tmp, **last = &nullobjRoot;
            for (i = 0; i < init_blocks; i++)
            {
                ptr = blockPointers[i];
                for (j = 0; j < blockSize; j++)
                {
                    tmp = &(ptr->data[j].ptr);
                    *last = (Type*)tmp;
                    last = tmp;
                }
            }
            *last = nullptr;
            nullobjBack = (Type*)last;
        }
        ObjectPool(const ObjectPool&) = delete;
        ObjectPool(ObjectPool&&) = default;
        ObjectPool& operator=(const ObjectPool&) = delete;
        ObjectPool& operator=(ObjectPool&&) = default;
        size_t size() const {return poolSize;}
        size_t capacity() const {return blockPointers.size()*blockSize;}
        Type* allocate()
        {
            if (nullobjRoot == nullobjBack)
            {
                alloc_block();
                return allocate();
            }
            else
            {
                poolSize++;
                Type* r = nullobjRoot;
                nullobjRoot = *(Type**)nullobjRoot;
                return r;
            }
        }
        void alloc_block()
        {
            obj_block* ptr = new obj_block();
            if (ptr == nullptr)
            {
                ERROR("MEMORY","内存耗尽!")
            }
            blockPointers.push_back(ptr);
            std::sort(blockPointers.begin(),blockPointers.end(),std::greater<obj_block*>());

            *(Type**)nullobjBack = (Type*)&(ptr->data[0].ptr);
            Type** tmp, **last = &(ptr->data[0].ptr);
            for (size_t j = 1; j < blockSize; j++)
            {
                tmp = &(ptr->data[j].ptr);
                *last = (Type*)tmp;
                last = tmp;
            }
            *last = nullptr;
            nullobjBack = (Type*)last;
        }
        void deallocate(Type* obj)
        {
            poolSize--;
            *(Type**)nullobjBack = obj;
            *(Type**)obj = nullptr;
            nullobjBack = obj;
        }
        ~ObjectPool()
        {
            for (size_t i = 0; i < blockPointers.size(); i++)
            {
                delete blockPointers[i];
            }
            nullobjRoot = nullptr;
        }
    };
}

#endif //!_BOUNDLESS_DATA_STRUCT_HPP_FILE_