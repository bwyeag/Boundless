#version 450 core
struct LightProperties {
    bool enable;
    bool isLocal;
    bool isSpot;
    vec3 ambient;
    vec3 color;
    vec3 position;
    vec3 halfVector;
    vec3 coneDirection;
    float spotCosCutoff;
    float spotExponent;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
};
const int MaxLights = 2;

uniform LightProperties Lights[MaxLights];
uniform vec3 eye_direction;
uniform float shininess;
uniform float strength;

in vec4 vcolor;
in vec3 vnormal;
in vec4 vposition;

out vec4 frag_color;

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
            lightDirection = lightDirection - vec3(vposition);
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
        float diffuse = max(0.0, dot(vnormal, lightDirection));
        float specular;
        if (diffuse == 0.0) {
            specular = 0.0;
        } else {
            specular = max(0.0,dot(vnormal, halfVector));
            specular = pow(specular, shininess) * strength;
        }
        scatteredLight += Lights[i].ambient * attenuation + Lights[i].color * diffuse * attenuation;
        reflectedLight += Lights[i].color * specular * attenuation;
    }
    vec3 rgb = min(vcolor.rgb * scatteredLight + reflectedLight, vec3(1.0));
    frag_color = vec4(rgb, vcolor.a);
}