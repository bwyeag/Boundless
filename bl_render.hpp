#ifndef _BL_RENDER_HPP_FILE_
#define _BL_RENDER_HPP_FILE_
#include "boundless.hpp"

namespace Boundless {
struct LightProperties {
    bool enable, isLocal, isSpot;
    Vector3f ambient, color, position, halfVector, coneDirection;
    float spotCosCutoff, spotExponent, constantAttenuation, linearAttenuation;
};
struct MaterialProperties {
    Vector3f emission, ambient, diffuse, specular;
    float shininess;
};

class ClassicLightBase {
   private:
    Program shader;
   public:
    ClassicLightBase();
    void Load() {
        shader.AddShader(".\\shader\\classic_shader_vertex.glsl",GL_VERTEX_SHADER);
        shader.AddShader(".\\shader\\classic_shader_fragment.glsl",GL_FRAGMENT_SHADER);
        shader.Link();
    }
    ~ClassicLightBase();
};

class ClassicLightRender : public RenderObject {
   public:
    ClassicLightRender();
    ~ClassicLightRender();
};

}  // namespace Boundless
#endif  //!_BL_RENDER_HPP_FILE_