#ifndef _BLUNDLESS_MESH_MAKER_HPP_FILE_
#define _BLUNDLESS_MESH_MAKER_HPP_FILE_
#include "boundless_base.hpp"
#include "bl_resource.hpp"

namespace Boundless {
class MeshMaker {
   public:
    // Make~()方法 按要求生成模型
    static void MakeCube(Mesh& mesh, float size, VertexData df);
    static void MakePlane(Mesh& mesh,
                          float size,
                          int xdiv,
                          int ydiv,
                          VertexData df);
    static void MakeSphere(Mesh& mesh,
                           float r,
                           int rdiv,
                           int hdiv,
                           VertexData df);
    // static void MakeTorus(Mesh& mesh,
    //                        float r,
    //                        float d,
    //                        int rdiv,
    //                        int hdiv,
    //                        VertexData df);
    static void MakeCubord(Mesh& mesh,
                           float a,
                           float b,
                           float c,
                           VertexData df);
};
}  // namespace Boundless

#endif  //!_BLUNDLESS_MESH_MAKER_HPP_FILE_