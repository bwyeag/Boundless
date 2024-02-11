#include "gl_render.hpp"

namespace Boundless::Render
{
    void RenderObject::Initialize(bool enable, const TransformDataPack& tdp, const Resource::MeshLoader& ml, const RenderFunctionPack& rfp)
    {
        this->enable = enable;
        transform.Initialize(tdp);
        glCreateVertexArrays(1, &info.vertex_array);
        const auto& buf = ml.GetBuffers();
        info.vectex_buffer = buf [0];

        draw_function = rfp.draw_function;
        destory_function = rfp.destory_function;
        switch (ml.GetIndexStatus())
        {
        case IndexStatus::NONE:
            rfp.create_function(&buf [1], buf.size() - 1, &info, ml.GetIndexStatus());
            break;
        case IndexStatus::INDEX_WITH_RESTARTINDEX:
            info.restart_index = static_cast<GLuint>(ml.GetRestartIndex());
            info.index_buffer = buf [1];
            rfp.create_function(&buf [2], buf.size() - 2, &info, ml.GetIndexStatus());
            break;
        case IndexStatus::ONLY_INDEX:
            info.index_buffer = ml.GetBuffers() [1];
            rfp.create_function(&buf [2], buf.size() - 2, &info, ml.GetIndexStatus());
            break;
        default:
            ERROR(RENDER_ERROR, RENDER_INDEX_STATUS_ERROR);
            Log::error_handle();
            return;
        }
    }
    void Initialize(bool enable, const TransformDataPack& tdp,Resource::mesh_pack& mp, const RenderFunctionPack& rfp)
    {
        this->enable = enable;
        transform.Initialize(tdp);
        glCreateVertexArrays(1, &info.vertex_array);
        glCreateBuffers(1, &info.vectex_buffer);
        glNamedBufferData(info.vectex_buffer, mp.vertex_size, mp.vertex_ptr, GL_STATIC_DRAW);
        glCreateBuffers(1, &info.index_buffer);
        glNamedBufferData(info.index_buffer,mp.index_size,mp.index_ptr,GL_STATIC_DRAW);
        glVertexArrayElementBuffer()
    }
    void RenderObject::Draw(const Matrix4x4& mat)
    {
        if (enable)
        {
            const Matrix4x4& model = transform.GetMatrix();
            draw_function(mat * model, this);
        }
    }
    void RenderObject::Destroy()
    {
        destory_function(&info);
        glDeleteVertexArrays(1, &info.vertex_array);
    }

    Render::Render(Camera& camera, GLFWwindow* rander_window)
        :render_datas(RENDER_INIT_BLOCKS)
    {
        this->camera = &camera;
        data_front = nullptr;
        this->rander_window = rander_window;
    }
    void Render::Draw()
    {
        glClearColor(RENDER_CLEAR_COLOR);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderDataPack* t = data_front;
        vp_mat = camera->GetProjection() * camera->GetView();
        while (t != nullptr)
        {
            t->d.Draw(vp_mat);
        }
        glfwSwapBuffers(rander_window);
    }
    RenderObject* Render::AddRenderObject(bool enable, const TransformDataPack& tdp, const Resource::MeshLoader& ml, const RenderFunctionPack& rfp)
    {
        RenderDataPack* ptrp = render_datas.allocate();
        ptrp->d.Initialize(enable, tdp,ml, rfp);
        return ptr;
    }
} // namespace Boundless::Render
