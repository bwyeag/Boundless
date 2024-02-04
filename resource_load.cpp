#include "resource_load.hpp"

namespace Boundless::Resource
{
    //模型加载函数,将模型数据加载到OpenGL
    ModelLoader::ModelLoader(const char* path, ModelLoadFunction f)
    {
        model_file.open(path,ios::binary|ios::in);
        {
            if (!model_file.is_open())
            {
                ERROR("Resource","无法打开模型文件:"<<path);
                exit(-1);
            }
            uint64_t head;
            model_file.read((char*)&head,sizeof(uint64_t));
            if (head!=MODEL_HEADER)
            {
                ERROR("Resource","模型文件类型错误,文件头代码:"<<head);
                exit(-1);
            }
        }
        ModelHead head_data;
        model_file.read((char*)&head_data,sizeof(ModelHead));
        GLsizei bcnt = head_data.buffer_count;
        has_other_buffers=(bcnt==0)?false:true;
        if (head_data.index_start==UINT64_MAX)
            has_index=false,bcnt+=1;
        else
            has_index=true,bcnt+=2;
        restart_index=head_data.restart_index;

        buffers.resize(bcnt);
        glCreateBuffers(bcnt,&buffers[0]);

        BufferInfoData bufferdata[head_data.buffer_count];
        if (head_data.buffer_count>0)
        {
            model_file.seekg(sizeof(uint64_t)+sizeof(ModelHead));
            model_file.read((char*)&bufferdata,head_data.buffer_count*sizeof(BufferInfoData));
        }

        if (f==nullptr)
        {
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
            for (uint32_t i = 0; i < head_data.buffer_count; i++)
            {
                glNamedBufferStorage(bptr[i],bufferdata[i].length,nullptr,GL_MAP_WRITE_BIT);
                char* wdata = (char*)glMapNamedBuffer(bptr[i],GL_WRITE_ONLY);
                model_file.seekg(bufferdata[i].start);
                model_file.read(wdata,bufferdata[i].length);
                glUnmapNamedBuffer(bptr[i]);
            }
            
        }
        else
        {
            f(this,&head_data,bufferdata);
        }
    }

    void ModelLoader::ReadFile(void* store_to, size_t start, size_t length)
    {
        model_file.seekg(start);
        model_file.read((char*)store_to, length);
    }

    TextureLoader::TextureLoader(const char* path, TextureLoadFunction f)
    {
        texture_file.open(path,ios::binary|ios::in);
        {
            if (!texture_file.is_open())
            {
                ERROR("Resource","无法打开纹理文件:"<<path);
                exit(-1);
            }
            uint64_t head;
            texture_file.read((char*)&head,sizeof(uint64_t));
            if (head!=TEXTURE_HEADER)
            {
                ERROR("Resource","纹理文件类型错误,文件头代码:"<<head);
                exit(-1);
            }
        }
        texture_file.read((char*)&texture_info,sizeof(TextureHead));
        glCreateTextures(texture_info.gl_target,1,&texture);
        int count = 1+(texture_info.height==0)?0:1+(texture_info.depth==0)?0:1;
        size_t start,length;
        start=texture_file.tellg();
        texture_file.seekg(ios::end);
        length=texture_file.tellg();
        length=length-start;
        void *tmp=malloc(length);
        if (tmp==0)
        {
            ERROR("Memory","内存耗尽");
            exit(-1);
        }
        texture_file.seekg(start);
        texture_file.read(tmp,length);
        switch (count)
        {
        case 1:
            glTextureStorage1D(texture,texture_info.mipmap_level,texture_info.gl_internal_format,texture_info.width);
            glTextureSubImage1D(texture,0,0,texture_info.width,texture_info.gl_format,texture_info.gl_type,tmp);
            break;
        case 2:
            glTextureStorage2D(texture,texture_info.mipmap_level,texture_info.gl_internal_format,texture_info.width,texture_info.height);
            glTextureSubImage2D(texture,0,0,0,texture_info.width,texture_info.height,texture_info.gl_format,texture_info.gl_type,tmp);
            break;
        case 3:
            glTextureStorage2D(texture,texture_info.mipmap_level,texture_info.gl_internal_format,texture_info.width,texture_info.height,texture_info.depth);
            glTextureSubImage3D(texture,0,0,0,0,texture_info.width,texture_info.height,texture_info.depth,texture_info.gl_format,texture_info.gl_type,tmp);
            break;
        default:
            ERROR("Resource","无法识别的纹理长宽高格式:"<<count);
            exit(-1);
        }
        if (f!=nullptr)
        {
            f(this,texture,&texture_info);
        }
        
    }
} // namespace Boundless::Resource