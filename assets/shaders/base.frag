#version 330 core

in vec3 v_worldPosition;
in vec3 v_normal;
in vec2 v_texCoord;
in vec3 v_tangent;

uniform vec3 u_materialColor;
uniform vec3 u_lightDirection;
uniform vec3 u_ambientColor;
uniform vec3 u_lightColor;
uniform vec3 u_fogColor;

out vec4 fragColor;

void main()
{
    vec3 normal = normalize(v_normal);
    vec3 tangent = normalize(v_tangent);
    normal = normalize(normal + tangent * 0.015);
    vec3 lightDirection = normalize(u_lightDirection);

    float diffuse = max(dot(normal, lightDirection), 0.0);
    float grime = 0.82 + 0.18 * sin(v_texCoord.x * 37.0 + v_texCoord.y * 19.0);

    vec3 albedo = u_materialColor * grime;
    vec3 lit = albedo * (u_ambientColor + diffuse * u_lightColor);

    float distanceFog = clamp(length(v_worldPosition) / 120.0, 0.0, 1.0);
    vec3 color = mix(lit, u_fogColor, distanceFog * 0.45);

    fragColor = vec4(color, 1.0);
}
