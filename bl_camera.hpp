#ifndef _CAMERA_HPP_FILE_
#define _CAMERA_HPP_FILE_

namespace Boundless::Render
{
    /// @brief 摄像机投影类型
    enum struct ViewType
    {
        PERSPECTIVE,
        ORTHOGONAL
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
        float znear, zfar;
        /// @brief 透视模式时的屏幕纵横比
        float pers_k;
        /// @brief 透视模式时的视角(弧度)(fov)
        float pers_fov;
        /// @brief 正交模式时的左/右/下/上位置
        float ortho_left, ortho_right, ortho_bottom, ortho_top;
        /// @brief 摄像机投影矩阵
        Matrix4x4 projection_mat;

    public:
        inline void ModificateView() { view_modificated = true; }

        inline void ModificateProj() { proj_modificated = true; }
        inline void SetViewType(ViewType vt) { type = vt; }
        inline void SetNearPlane(float n) { znear = n; }
        inline void SetFarPlane(float n) { zfar = n; }
        inline void SetRatio(float h, float w) { pers_k = h / w; }

        inline Vector3 GetPosition() const { return position; }
        inline const Matrix4x4 &GetView()
        {
            if (view_modificated)
            {
                view_mat = glm::lookAt(position, position + dir_front, dir_up);
                view_modificated = false;
            }
            return view_mat;
        }

        inline const Matrix4x4 &GetProjection()
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
} // namespace Boundless::Render

#endif //!_CAMERA_HPP_FILE_