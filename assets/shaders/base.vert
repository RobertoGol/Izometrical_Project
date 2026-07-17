#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texCoord;
layout (location = 3) in vec3 a_tangent;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 v_worldPosition;
out vec3 v_normal;
out vec2 v_texCoord;
out vec3 v_tangent;

void main()
{
    vec4 worldPosition = u_model * vec4(a_position, 1.0);
    v_worldPosition = worldPosition.xyz;
    v_normal = mat3(transpose(inverse(u_model))) * a_normal;
    v_tangent = mat3(u_model) * a_tangent;
    v_texCoord = a_texCoord;

    gl_Position = u_projection * u_view * worldPosition;
}
