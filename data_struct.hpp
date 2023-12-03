/*
 * BOUNDLESS Engine File
 * Create: 2023/12/3/10:56
 * Data Struct C++ Header
 *
 */
#ifndef _BOUNDLESS_DATA_STRUCT_HPP_FILE_
#define _BOUNDLESS_DATA_STRUCT_HPP_FILE_
#include <cstdint>
#include <cstddef>

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
namespace Boundless
{
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
            init_blocks = max(init_blocks,(size_t)1);
            obj_block* ptr;
            blockPointers.resize(init_blocks);
            size_t i,j;
            for (i = 0; i < init_blocks; i++)
            {
                ptr = new obj_block();
                if (ptr == nullptr)
                {
                    ERROR("MEMORY","内存耗尽!")
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
    template<typename Data>
    class ArrayBinTree
    {
    private:
        std::vector<Data> data;
    public:
        using Handle = size_t;
        BinTree(){}
        ~BinTree(){}
        BinNode* create_root(Data* data)
        {
            assert(data.empty());
            treeRoot = nodePool.allocate();
            treeRoot->parent = nullptr;
            treeRoot->leftChild = nullptr;
            treeRoot->rightChild = nullptr;
            treeRoot->nodeData = *data;
            return treeRoot;
        }
        BinNode* create_left(BinNode* base, Data* data)
        {
            assert(base->leftChild==nullptr);
            BinNode* r = nodePool.allocate();
            base->leftChild = r;
            r->parent = base;
            r->leftChild = nullptr;
            r->rightChild = nullptr;
            r->nodeData = *data;
            return r;
        }
        BinNode* create_right(BinNode* base, Data* data)
        {
            assert(base->rightChild==nullptr);
            BinNode* r = nodePool.allocate();
            base->rightChild = r;
            r->parent = base;
            r->leftChild = nullptr;
            r->rightChild = nullptr;
            r->nodeData = *data;
            return r;
        }
        void delete_node(BinNode* base)
        {
            if (base->parent->rightChild == base)
            {
                base->parent->rightChild = nullptr;
            }
            else
            {
                base->parent->leftChild = nullptr;
            }
            pre_iteration([&nodePool](BinNode* n){
                nodePool.deallocate(n);
            },base);
        }
        size_t degree(BinNode* n)
        {
            size_t r = 0;
            if (n->leftChild!=nullptr)
            {
                r++;
            }
            if (n->rightChild!=nullptr)
            {
                r++;
            }
            return r;
        }
        void pre_iteration(std::function<void(BinNode*)> f,BinNode* n)
        {
            std::stack<BinNode*> indstack;
            indstack.push(n);
            while (!indstack.empty())
            {
                n = indstack.top();
                indstack.pop();
                f(n);
                if (n->leftChild != nullptr)
                {
                    indstack.push(n->leftChild);
                }
                if (n->rightChild != nullptr)
                {
                    indstack.push(n->rightChild);
                }
            }
        }
        void mid_iteration(std::function<void(BinNode*)> f,BinNode* n)
        {
            std::stack<BinNode*> indstack;
            while (!indstack.empty() || p)
            {
                while (p)
                {
                    indstack.push(p);
                    p = p->leftChild;
                }
                if (!indstack.empty())
                {
                    p = indstack.top();
                    indstack.pop();
                    f(p);
                    p = p->rightChild;
                }
            }
        }
        void post_iteration(std::function<void(BinNode*)> f,BinNode* n)
        {
            std::stack<BinNode*> indstack,dostack;
            indstack.push(n);
            while (!indstack.empty())
            {
                n = indstack.top();
                indstack.pop();
                dostack.push(n);
                if (n->rightChild != nullptr)
                {
                    indstack.push(n->rightChild);
                }
                if (n->leftChild != nullptr)
                {
                    indstack.push(n->leftChild);
                }            
            }
            while (!dostack.empty())
            {
                f(dostack.top());
                dostack.pop();
            }
        }
    };
    template<typename Data, size_t NODE_BLOCK_SIZE>
    class BinTree
    {
    public:
        struct BinNode
        {
            BinNode* parent,leftChild,rightChild;
            Data nodeData;
        };
    private:
        ObjectPool<BinNode,NODE_BLOCK_SIZE> nodePool;
        BinNode* treeRoot = nullptr;
    public:
        BinTree(){}
        ~BinTree(){}
        BinNode* create_root(Data* data)
        {
            assert(treeRoot==nullptr);
            treeRoot = nodePool.allocate();
            treeRoot->parent = nullptr;
            treeRoot->leftChild = nullptr;
            treeRoot->rightChild = nullptr;
            treeRoot->nodeData = *data;
            return treeRoot;
        }
        BinNode* create_left(BinNode* base, Data* data)
        {
            assert(base->leftChild==nullptr);
            BinNode* r = nodePool.allocate();
            base->leftChild = r;
            r->parent = base;
            r->leftChild = nullptr;
            r->rightChild = nullptr;
            r->nodeData = *data;
            return r;
        }
        BinNode* create_right(BinNode* base, Data* data)
        {
            assert(base->rightChild==nullptr);
            BinNode* r = nodePool.allocate();
            base->rightChild = r;
            r->parent = base;
            r->leftChild = nullptr;
            r->rightChild = nullptr;
            r->nodeData = *data;
            return r;
        }
        void delete_node(BinNode* base)
        {
            if (base->parent->rightChild == base)
            {
                base->parent->rightChild = nullptr;
            }
            else
            {
                base->parent->leftChild = nullptr;
            }
            pre_iteration([&nodePool](BinNode* n){
                nodePool.deallocate(n);
            },base);
        }
        size_t degree(BinNode* n)
        {
            size_t r = 0;
            if (n->leftChild!=nullptr)
            {
                r++;
            }
            if (n->rightChild!=nullptr)
            {
                r++;
            }
            return r;
        }
        void pre_iteration(std::function<void(BinNode*)> f,BinNode* n)
        {
            std::stack<BinNode*> indstack;
            indstack.push(n);
            while (!indstack.empty())
            {
                n = indstack.top();
                indstack.pop();
                f(n);
                if (n->leftChild != nullptr)
                {
                    indstack.push(n->leftChild);
                }
                if (n->rightChild != nullptr)
                {
                    indstack.push(n->rightChild);
                }
            }
        }
        void mid_iteration(std::function<void(BinNode*)> f,BinNode* n)
        {
            std::stack<BinNode*> indstack;
            while (!indstack.empty() || p)
            {
                while (p)
                {
                    indstack.push(p);
                    p = p->leftChild;
                }
                if (!indstack.empty())
                {
                    p = indstack.top();
                    indstack.pop();
                    f(p);
                    p = p->rightChild;
                }
            }
        }
        void post_iteration(std::function<void(BinNode*)> f,BinNode* n)
        {
            std::stack<BinNode*> indstack,dostack;
            indstack.push(n);
            while (!indstack.empty())
            {
                n = indstack.top();
                indstack.pop();
                dostack.push(n);
                if (n->rightChild != nullptr)
                {
                    indstack.push(n->rightChild);
                }
                if (n->leftChild != nullptr)
                {
                    indstack.push(n->leftChild);
                }            
            }
            while (!dostack.empty())
            {
                f(dostack.top());
                dostack.pop();
            }
        }
    };
    template<typename Data, size_t NODE_BLOCK_SIZE>
    class BasicTree
    {
    public:
        struct BasicNode
        {
            BasicNode* parent;
            std::list<BasicNode*> children;
            Data nodeData;
        };
    private:
        ObjectPool<BasicNode,NODE_BLOCK_SIZE> nodePool;
        BasicNode* treeRoot;
    public:
        BasicTree(){}
        ~BasicTree(){}
        BasicNode* create_root(Data* data)
        {
            treeRoot = nodePool.allocate();
            treeRoot->parent = nullptr;
            new(&r->children) std::list<BasicNode*>();
            treeRoot->nodeData = *data;
            return treeRoot;
        }
        BasicNode* create_left(BasicNode* base, Data* data)
        {
            BinNode* r = nodePool.allocate();
            base->children.push_front(r);
            r->parent = base;
            new(&r->children) std::list<BasicNode*>();
            r->nodeData = *data;
            return r;
        }
        BasicNode* create_right(BasicNode* base, Data* data)
        {
            BinNode* r = nodePool.allocate();
            base->children.push_back(r);
            r->parent = base;
            new(&r->children) std::list<BasicNode*>();
            r->nodeData = *data;
            return r;
        }
        void delete_node(BasicNode* base)
        {
            std::list<Index>& chs = base->parent->children;
            for (auto i = chs.begin(); i != chs.end(); i++)
            {
                if (*i==base)
                {
                    chs.erase(i);
                    break;
                }
            }
            pre_iteration([&nodePool](BasicNode* n){
                n->children.clear();
                nodePool.deallocate(n);
            },base);
        }
        size_t degree(BasicNode* n)
        {
            return n->children.size();
        }
        void pre_iteration(std::function<void(BasicNode*)> f,BasicNode* n)
        {
            std::stack<BinNode*> indstack;
            indstack.push(n);
            while (!indstack.empty())
            {
                n = indstack.top();
                indstack.pop();
                f(n);
                for (auto i = n->children.begin(); i != n->children.end(); i++)
                {
                    indstack.push(*i);
                }
            }
        }
        void post_iteration(std::function<void(BasicNode*)> f,BasicNode* n)
        {
            std::stack<BinNode*> indstack,dostack;
            indstack.push(n);
            while (!indstack.empty())
            {
                n = indstack.top();
                indstack.pop();
                dostack.push(n);
                for (auto i = n->children.rbegin(); i != n->children.rend(); i++)
                {
                    indstack.push(*i);
                }
            }
            while (!dostack.empty())
            {
                f(dostack.top());
                dostack.pop();
            }
        }
    };
}

#endif //!_BOUNDLESS_DATA_STRUCT_HPP_FILE_