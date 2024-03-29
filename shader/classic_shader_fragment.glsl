#version 450 core
struct LightProp {
    Vector3f ambient;  // true->锥光;0
    Vector3f color;    // 颜色;16
    Vector3f position;  // if (isLocal) 表示光的位置; else 表示光的方向;32
    Vector3f halfVector;  // 锥光的半程向量;48
    Vector3f coneDirection; //;64
    bool enable;       // 是否启用;76
    bool isLocal;      // true->点光源和锥光; false->方向光;77
    bool isSpot;       // true->锥光;78
    float spotCosCutoff;         // 聚光灯余弦截止;80
    float spotExponent;          // 聚光灯衰减系数;84
    float constantAttenuation;   // 光照衰减常量部分;88
    float linearAttenuation;     // 光照衰减线性部分;92
    float quadraticAttenuation;  // 光照衰减平方部分;96
};
struct MaterialProp {
    vec3 emission; // 材质的照明;0
    vec3 ambient;   //;16
    vec3 diffuse;   //;32
    vec3 specular;  //;48
    float shininess;    //;60
};
const int MaxLights = 2;
const int MaxMaterials = 1;

layout(std140, binding = 0) uniform Data {
    LightProp Lights[MaxLights];
    MaterialProp Materials[MaxMaterials];
};
layout(location = 6) uniform vec3 EyeDirection;
layout(location = 7) uniform int MaterialIndex;

in vec4 Color;
in vec3 Normal;
in vec4 WorldPosition;

out vec4 FragColor;

void main() {
    vec3 scatteredLight = vec3(0.0);
    vec3 reflectedLight = vec3(0.0);

    for (int i = 0;i<MaxLights;++i) {
        if (!Lights[i].enable)
            continue;
        vec3 halfVector;
        vec3 lightDirection = Lights[i].position;
        float attenuation = 1.0;
        if (Lights[i].isLocal) {
            lightDirection = lightDirection - vec3(WorldPosition);
            float lightDistance = length(lightDirection);
            lightDirection /= lightDistance;
            attenuation = 1.0 / (Lights[i].constantAttenuation + Lights[i].linearAttenuation * lightDistance + Lights[i].quadraticAttenuation * lightDistance * lightDistance);
            if (Lights[i].isSpot) {
                float spotCos = dot(lightDirection, Lights[i].coneDirection);
                if (spotCos < Lights[i].spotCosCutoff)
                    attenuation = 0.0;
                else 
                    attenuation *= pow(spotCos, Lights[i].spotExponent);
            }
            halfVector = normalize(lightDirection+eye_direction);
        } else {
            halfVector = Lights[i].halfVector;
        }
        float diffuse = max(0.0, dot(Normal, lightDirection));
        float specular;
        if (diffuse == 0.0) {
            specular = 0.0;
        } else {
            specular = max(0.0,dot(Normal, halfVector));
            specular = pow(specular, shininess) * strength;
        }
        scatteredLight += Lights[i].ambient * Materials[MaterialIndex].ambient * attenuation + Lights[i].color * Materials[MaterialIndex].diffuse *  diffuse * attenuation;
        reflectedLight += Lights[i].color * Materials[MaterialIndex].specular * specular * attenuation;
    }
    vec3 rgb = min(Materials[MaterialIndex].emission + Color.rgb * scatteredLight + reflectedLight, vec3(1.0));
    FragColor = vec4(rgb, Color.a);
}