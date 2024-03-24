#version 450 core
layout(location = 0) uniform mat4 MVPMatrix;
layout(location = 1) uniform mat4 MVMatrix;
layout(location = 2) uniform mat3 NormalMatrix;

layout(location = 3) uniform VertexColor;

in vec3 VertexPosition;
in vec3 VertexNormal;

out vec4 Color;
out vec3 Normal;
out vec4 WorldPosition;

void main() {
    Color = VertexColor;
    Normal = normalize(NormalMatrix * VertexNormal);
    WorldPosition = MVMatrix * VertexPosition;
    gl_Position = MVPMatrix * VertexPosition;
}