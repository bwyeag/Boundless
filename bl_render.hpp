#ifndef _RENDER_HPP_FILE_
#define _RENDER_HPP_FILE_
#include "glad/glad.h"
#include "glfw/glfw3.h"

#include "bl_data_struct.hpp"
#include "bl_resource_load.hpp"
#include "bl_log.hpp"
#include "bl_mesh.hpp"
#include "bl_camera.hpp"

namespace Boundless::Render
{
    /// @brief 传递模型数据
    struct TransformDataPack
    {
        /// @brief 位置
        Vector3 position;
        /// @brief 缩放
        Vector3 scale;
        /// @brief 旋转(四元数)
        Quaternion rotation;
    };

    /// @brief 模型变换类
    class Transform
    {
    private:
        /// @brief 记录当前修改状态
        mutable bool modificated;
        /// @brief 位置
        Vector3 position;
        /// @brief 缩放
        Vector3 scale;
        /// @brief 旋转
        Quaternion rotation;
        /// @brief
        mutable Matrix4x4 model_mat;

    public:
        Transform() {}
        /// @param p 位置
        /// @param s 缩放
        /// @param q 旋转
        Transform(const Vector3 &p, const Vector3 &s, const Quaternion &q)
            : modificated(true), position(p), scale(s), rotation(q)
        {
        }
        /// @param tdp 模型数据
        Transform(const TransformDataPack &tdp)
            : modificated(true), position(tdp.position), scale(tdp.scale), rotation(tdp.rotation)
        {
        }
        Transform(Transform &&) = default;
        Transform(const Transform &) = default;
        Transform &operator=(Transform &&) = default;
        Transform &operator=(const Transform &) = default;

        /// @brief 初始化(可代替构造函数)
        /// @param tdp 初始化数据
        void Initialize(const TransformDataPack &tdp);

        /// @brief 标记修改状态(在修改之前调用)
        inline void Modificate() const { modificated = true; }
        inline void SetPosition(const Vector3 &n) { position = n; }
        inline void SetScale(const Vector3 &n) { scale = n; }
        inline void SetRotation(const Quaternion &n) { rotation = n; }
        inline void SetRotation(const Vector3 &eulerAngle) { rotation = Quaternion(eulerAngle); }

        inline Vector3 GetPosition() const { return position; }
        inline Vector3 GetScale() const { return scale; }
        inline Quaternion GetRotation() const { return rotation; }

        inline const Matrix4x4 &GetMatrix() const
        {
            if (modificated)
            {
                model_mat = Matrix4x4(static_cast<float>(1.0f));
                model_mat = glm::scale(model_mat, scale);
                model_mat = glm::mat4_cast(rotation) * model_mat;
                model_mat = glm::translate(model_mat, position);
                modificated = false;
            }
            return model_mat;
        }
    };

    /// @brief 渲染对象创建时调用的函数
    typedef void (*RenderCreateFunction)(Mesh &mesh, void *fp);
    /// @brief 渲染绘制函数
    typedef void (*RenderDrawFunction)(const Matrix4x4 &matrix, Mesh &mesh, void *fp);
    /// @brief 渲染对象销毁时调用的函数
    typedef void (*RenderDestoryFunction)(Mesh &mesh, void *fp);

    /// @brief 渲染函数的包装
    struct RenderFunctionPack
    {
        RenderCreateFunction create_function;
        RenderDrawFunction draw_function;
        RenderDestoryFunction destory_function;
    };
    using Resource::IndexStatus;
    /// @brief 渲染对象
    class RenderObject
    {
    private:
        bool enable;
        Transform transform;
        Mesh mesh;

        RenderDrawFunction draw_function;
        RenderDestoryFunction destory_function;
        void *function_pointer;

    public:
        /// @brief 启用渲染
        inline void Enable() { enable = true; }
        /// @brief 禁用渲染
        inline void Disable() { enable = false; }
        /// @brief 获取渲染对象模型变换的数据
        inline Transform &GetTransform() { return transform; }
        /// @brief 初始化渲染
        /// @param enable 是否启用
        /// @param tdp 模型变换数据
        /// @param ml 渲染的模型网格
        /// @param rfp 渲染函数包装
        void Initialize(bool enable, const TransformDataPack &tdp, const Resource::MeshLoader &ml, const RenderFunctionPack &, void *fp);
        void Initialize(bool enable, const TransformDataPack &tdp, Mesh *mp, const RenderFunctionPack &rfp, void *fp);
        /// @brief 绘制调用
        /// @param mat 投影-视图矩阵
        void Draw(const Matrix4x4 &mat);
        /// @brief 销毁调用
        void Destroy();
    };

#define RENDER_INIT_BLOCKS 8
#define RENDER_CLEAR_COLOR 0.0f, 0.0f, 0.0f, 1.0f
    class Renderer
    {
    private:
        struct RenderDataPack
        {
            RenderDataPack *next;
            RenderObject d;
        };
        ObjectPool<RenderDataPack> render_datas;
        RenderDataPack *data_front;
        Camera *camera;
        GLFWwindow *rander_window;
        Matrix4x4 vp_mat;

    public:
        Renderer(Camera *camera, GLFWwindow *rander_window);
        Renderer(Renderer &&) = default;
        Renderer(const Renderer &) = delete;
        Renderer &operator=(Renderer &&) = default;
        Renderer &operator=(const Renderer &) = delete;
        /*
         * 绘制当前帧图像
         * 在调用前确保已调用glfwMakeContextCurrent()函数，设置为rander_window
         */

        void Draw();
        template <typename... Types>
        RenderObject *AddRenderObject(Types &...args)
        {
            RenderDataPack *ptrp = render_datas.allocate();
            if constexpr (sizeof...(args) > 0)
            {
                ptrp->d.Initialize(args...);
            }
            return &ptrp->d;
        }

        inline void UseCamera(Camera *camera) { this->camera = camera; }
    };

} // namespace Boundless::Render
#endif //!_RENDER_HPP_FILE_