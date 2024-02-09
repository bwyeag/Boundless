#include "render.hpp"

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

    Render::Render(CameraData* camera, GLFWwindow* rander_window)
        :render_datas(RENDER_INIT_BLOCKS)
    {
        this->camera = camera;
        data_front = nullptr;
        this->rander_window = rander_window;
    }
    void Render::Draw()
    {
        glClearColor(RENDER_CLEAR_COLOR);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderDataPack* t = data_front;
        camera->UpdateProjection();
        camera->UpdateView();
        vp_mat = camera->projection_mat * camera->view_mat;
        while (t != nullptr)
        {
            if (t->d.enable)
            {
                t->d.transform.UpdateMatrix();
                t->d.draw_function(&(t->d), vp_mat);
            }
        }
        glfwSwapBuffers(rander_window);
    }
    RenderData* Render::AddRenderObject(bool enable, const MeshLoader& ml, const Transform& tr, RenderCreateFunction rcf, RenderDrawFunction drawf, RenderDestoryFunction destf)
    {
        RenderDataPack* ptrp = render_datas.allocate();
        RenderData* ptr = &ptrp->d;
        ptr->enable = enable;
        ptr->transform = tr;
        ptr->transform.modificated = true;
        glCreateVertexArrays(1, &ptr->vertex_array);
        vectex_buffer = ml.GetBuffers() [0];
        if (ml.HasIndex())
        {
            ptr->index_array = ml.GetBuffers() [1];
        }
        else
        {
            ptr->index_array = 0;
        }
        ptr->draw_function = drawf;
        ptr->destory_function = destf;
        if (ml.HasOtherBuffers())
        {
            size_t st = ml.has_index() ? 2 : 1;
            rcf(ptr, &ml.GetBuffers() [st], ml.GetBuffers().size() - st, ml.GetRestartIndex());
        }
        else
        {
            rcf(ptr, nullptr, 0ULL, ml.GetRestartIndex());
        }
        if (data_front == nullptr)
        {
            data_front = ptrp;
            ptrp->next = nullptr;
        }
        else
        {
            ptrp->next = data_front;
            data_front = ptrp
        }
        return ptr;
    }
} // namespace Boundless::Render
