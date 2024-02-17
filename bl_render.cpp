#include "bl_render.hpp"

namespace Boundless::Render
{
    void Transform::Initialize(const TransformDataPack &tdp)
    {
        modificated = true;
        position = tdp.position;
        scale = tdp.scale;
        rotation = tdp.rotation;
    }
    void RenderObject::Initialize(bool enable, const TransformDataPack &tdp, const Resource::MeshLoader &ml, const RenderFunctionPack &rfp, void *fp)
    {
        function_pointer = fp;
        this->enable = enable;
        transform.Initialize(tdp);
        mesh.Initialize(ml);
        draw_function = rfp.draw_function;
        destory_function = rfp.destory_function;
        rfp.create_function(mesh, fp);
    }
    void RenderObject::Initialize(bool enable, const TransformDataPack &tdp, Mesh *meshptr, const RenderFunctionPack &rfp, void *fp)
    {
        function_pointer = fp;
        this->enable = enable;
        transform.Initialize(tdp);
        if (meshptr != nullptr)
        {
            mesh = *meshptr;
        }
        draw_function = rfp.draw_function;
        destory_function = rfp.destory_function;
        rfp.create_function(mesh, fp);
    }
    void RenderObject::Draw(const Matrix4x4 &mat)
    {
        if (enable)
        {
            const Matrix4x4 &model = transform.GetMatrix();
            draw_function(mat * model, mesh, function_pointer);
        }
    }
    void RenderObject::Destroy()
    {
        destory_function(mesh, function_pointer);
    }

    Renderer::Renderer(Camera *camera, GLFWwindow *rander_window)
        : render_datas(RENDER_INIT_BLOCKS)
    {
        this->camera = camera;
        data_front = nullptr;
        this->rander_window = rander_window;
    }
    void Renderer::Draw()
    {
        glClearColor(RENDER_CLEAR_COLOR);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderDataPack *t = data_front;
        vp_mat = camera->GetProjection() * camera->GetView();
        while (t != nullptr)
        {
            t->d.Draw(vp_mat);
        }
        glfwSwapBuffers(rander_window);
    }
} // namespace Boundless::Render
