#version 450 core
uniform mat4 MVP_matrix;
uniform mat4 MV_matrix;
uniform mat4 normal_matrix;
uniform mat4 color;

in vec3 vertex_position;
in vec3 vertex_normal;

out vec4 vcolor;
out vec3 vnormal;
out vec4 vposition;

void main() {
    vcolor = color;
    vnormal = normalize(normal_matrix * vertex_normal);
    vposition = MV_matrix * vertex_position;
    gl_Position = MVP_matrix * vertex_position;
}