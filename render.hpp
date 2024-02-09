#ifndef _RENDER_HPP_FILE_
#define _RENDER_HPP_FILE_
#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>

#include "data_struct.hpp"
#include "resource_load.hpp"
#include "error_handle.hpp"

namespace Boundless::Render
{
#ifndef BOUNDLESS_USE_DOUBLE 
    using Real = float;
    using Vector3 = glm::vec3;
    using Quaternion = glm::quat;
    using Matrix4x4 = glm::mat4;
#else
    using Real = double;
    using Vector3 = glm::dvec3;
    using Quaternion = glm::dquat;
    using Matrix4x4 = glm::dmat4;
#endif
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
        bool modificated;
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
        Transform(const Vector3& p, const Vector3& s, const Quaternion& q)
            : modificated(true), position(p), scale(s), rotation(q)
        {}
        /// @param tdp 模型数据
        Transform(const TransformDataPack& tdp)
            : modificated(true), position(tdp.position), scale(tdp.scale), rotation(tdp.rotation)
        {}
        Transform(Transform&&) = default;
        Transform(const Transform&) = default;
        Transform& operator=(Transform&&) = default;
        Transform& operator=(const Transform&) = default;

        /// @brief 初始化(可代替构造函数)
        /// @param tdp 初始化数据
        void Initialize(const TransformDataPack& tdp)
        {
            modificated = true;
            position = tdp.position;
            scale = tdp.scale;
            rotation = tdp.rotation;
        }

        /// @brief 标记修改状态(在修改之前调用)
        inline void Modificate() { modificated = true; }
        inline void SetPosition(const Vector3& n) { position = n; }
        inline void SetScale(const Vector3& n) { scale = n; }   
        inline void SetRotation(const Quaternion& n) { rotation = n; }
        inline void SetRotation(const Vector3& eulerAngle)
        {
            rotation = Quaternion(eulerAngle);
        }

        inline Vector3 GetPosition() const { return position; }
        inline Vector3 GetScale() const { return scale; }
        inline Quaternion GetRotation() const { return rotation; }

        inline const Matrix4x4& GetMatrix() const
        {
            if (modificated)
            {
                model_mat = Matrix4x4(static_cast<Real>(1.0f));
                model_mat = glm::scale(model_mat, scale);
                model_mat = glm::mat4_cast(rotation) * model_mat;
                model_mat = glm::translate(model_mat, position);
                modificated = false;
            }
            return model_mat;
        }
    };

    /// @brief 渲染所用OpenGL缓存对象/顶点数组对象包装
    struct RenderInfoHead
    {
        /// @brief 顶点数组对象
        GLuint vertex_array;
        /// @brief 顶点数据缓存
        GLuint vectex_buffer;
        /// @brief 索引数据缓存
        GLuint index_buffer;
        /// @brief 图元重启动索引
        GLuint restart_index;
    }
    /// @brief 渲染对象创建时调用的函数
    typedef void (*RenderCreateFunction)(const GLuint* buffers_pointer, size_t buffer_count, RenderInfoHead* data, Resource::IndexStatus st);
    /// @brief 渲染绘制函数
    typedef void (*RenderDrawFunction)(const Matrix4x4&, RenderInfoHead*);
    /// @brief 渲染对象销毁时调用的函数
    typedef void (*RenderDestoryFunction)(RenderInfoHead*);

    /// @brief 渲染函数的包装
    struct RenderFunctionPack
    {
        RenderCreateFunction create_function;
        RenderDrawFunction draw_function;
        RenderDestoryFunction destory_function;
    };
    /// @brief 渲染对象
    class RenderObject
    {
    private:
        bool enable;
        Resource::IndexStatus index_status;
        Transform transform;
        RenderInfoHead info;

        RenderDrawFunction draw_function;
        RenderDestoryFunction destory_function;
    public:
        /// @brief 启用渲染
        inline void Enable() { enable = true; }
        /// @brief 禁用渲染
        inline void Disable() { enable = false; }
        /// @brief 获取渲染对象模型变换的数据
        inline Transform& GetTransform() { return transform; }
        /// @brief 初始化渲染
        /// @param enable 是否启用
        /// @param tdp 模型变换数据
        /// @param ml 渲染的模型网格
        /// @param rfp 渲染函数包装
        void Initialize(bool enable, const TransformDataPack& tdp, const Resource::MeshLoader& ml, const RenderFunctionPack& rfp);
        /// @brief 绘制调用
        /// @param mat 投影-视图矩阵
        void Draw(const Matrix4x4& mat);
        /// @brief 销毁调用
        void Destroy();
    };
    /// @brief 摄像机投影类型
    enum struct ViewType
    {
        PERSPECTIVE, ORTHOGONAL
    };
    /// @brief 摄像机
    class Camera
    {
    private:
        bool view_modificated, proj_modificated;

        /// @brief 摄像机位置
        Vector3 position;
        /// @brief 摄像机上/前向量
        Vector3 dir_up, dir_front;
        /// @brief 摄像机视图矩阵
        Matrix4x4 view_mat;

        /// @brief 摄像机投影类型(透视/正交)
        ViewType type;
        /// @brief 视锥体近平面和远平面距离
        Real znear, zfar;
        /// @brief 透视模式时的屏幕纵横比
        Real pers_k;
        /// @brief 透视模式时的视角(弧度)(fov)
        Real pers_fov;
        /// @brief 正交模式时的左/右/下/上位置
        Real ortho_left, ortho_right, ortho_bottom, ortho_top;
        /// @brief 摄像机投影矩阵
        Matrix4x4 projection_mat;
    public:
        inline void ModificateView() { view_modificated = true; }
        inline void ModificateProj() { proj_modificated = true; }

        inline Vector3 GetPosition() const { return position; }
        inline void 
        inline const Matrix4x4& GetView()
        {
            if (view_modificated)
            {
                view_mat = glm::lookAt(position, position + dir_front, dir_up);
                view_modificated = false;
            }
            return view_mat;
        }

        inline const Matrix4x4& GetProjection()
        {
            if (proj_modificated)
            {
                if (type == ViewType::PERSPECTIVE)
                {
                    projection_mat = glm::perspective(perfov, perk, znear, zfar);
                }
                else
                {
                    projection_mat = glm::ortho(ortleft, ortright, ortbottom, orttop, znear, zfar);
                }
                proj_modificated = false;
            }
            return projection_mat;
        }
    };
#define RENDER_INIT_BLOCKS 32
#define RENDER_CLEAR_COLOR 0.0f, 0.0f, 0.0f, 1.0f
    struct RenderDataPack
    {
        RenderDataPack* next;
        RenderData d;
    };
    class Render
    {
    private:
        using namespace Boundless::Resource;
        ObjectPool<RenderDataPack> render_datas;
        RenderDataPack* data_front;
        CameraData* camera;
        GLFWwindow* rander_window;
        Matrix4x4 vp_mat;
    public:
        Render(CameraData* camera, GLFWwindow* rander_window);
        Render(Render&&) = default;
        Render(const Render&) = delete;
        Render& operator=(Render&&) = default;
        Render& operator=(const Render&) = delete;
        /*
        * 绘制当前帧图像
        * 在调用前确保已调用glfwMakeContextCurrent()函数，设置为rander_window
        */
        void Draw();
        RenderData* AddRenderObject(bool enable, const MeshLoader& ml, const Transform& tr, RenderCreateFunction rcf, RenderDrawFunction drawf, RenderDestoryFunction destf)
            inline void UseCamera(Camera* camera)
        {
            this->camera = camera;
        }
        ~Render();
    };

} // namespace Boundless::Render
#endif //!_RENDER_HPP_FILE_