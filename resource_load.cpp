#include "resource_load.hpp"

namespace Boundless::Resource
{
    //模型加载函数,将模型数据加载到OpenGL
    ModelLoader::ModelLoader(const char* path)
    {
        model_file.open(path,ios::binary|ios::in);
        {
            uint64_t head;
            model_file.read((char*)&head,sizeof(uint64_t));
            if (head!=MODEL_HEADER)
            {
                ERROR("Resource","文件类型错误,head:"<<head);
                exit(-1);
            }
        }
        model_file.read((char*)&head_data,sizeof(ModelHead));
        if (head_data.index_start==UINT64_MAX)
        {
            has_index=false;
        }
        GLsizei bcnt=head_data.buffer_count+has_index?2:1;
        glCreateBuffers(bcnt,&buffers[0]);

        if (head_data.signal==0)
        {
            buffers.resize(bcnt);
            glCreateBuffers(bcnt,&buffers[0]);

            glNamedBufferStorage(buffers[0],head_data.vertex_length,nullptr,GL_MAP_WRITE_BIT);
            char* wdata = (char*)glMapNamedBuffer(buffers[0],GL_WRITE_ONLY);
            model_file.seekg(head_data.vertex_start);
            model_file.read(wdata,head_data.vertex_length);
            glUnmapNamedBuffer(buffers[0]);

            if (has_index)
            {
                glNamedBufferStorage(buffers[1],head_data.index_length,nullptr,GL_MAP_WRITE_BIT);
                char* wdata = (char*)glMapNamedBuffer(buffers[1],GL_WRITE_ONLY);
                model_file.seekg(head_data.index_start);
                model_file.read(wdata,head_data.index_length);
                glUnmapNamedBuffer(buffers[1]);
            }

            GLuint* bptr=&buffers[has_index?2:1];
            BufferInfoData bdata[head_data.buffer_count];
            model_file.seekg(sizeof(uint64_t)+sizeof(ModelHead));
            model_file.read((char*)&bdata,head_data.buffer_count*sizeof(BufferInfoData));
            for (uint32_t i = 0; i < head_data.buffer_count; i++)
            {
                glNamedBufferStorage(bptr[i],bdata[i].length,nullptr,GL_MAP_WRITE_BIT);
                char* wdata = (char*)glMapNamedBuffer(bptr[i],GL_WRITE_ONLY);
                model_file.seekg(bdata[i].start);
                model_file.read(wdata,bdata[i].length);
                glUnmapNamedBuffer(bptr[i]);
            }
        }
        else if (auto search = load_functions.find(head_data.signal); search != load_functions.end())
        {
            model_file.seekg(ios::beg);
            (*search)(this);
        }
        else
        {
            ERROR("Resource","指定的预处理函数无法找到:" << head_data.signal);
        }
    }
    ModelLoader::ModelLoader(string path)
        : ModelLoader(path.c_str()) {}
} // namespace Boundless::Resource