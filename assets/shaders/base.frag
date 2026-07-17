#version 330 core

in vec3 v_worldPosition;
in vec3 v_normal;
in vec2 v_texCoord;

uniform vec3 u_materialColor;

out vec4 fragColor;

void main()
{
    vec3 normal = normalize(v_normal);
    vec3 lightDirection = normalize(vec3(-0.35, 0.85, 0.28));

    float diffuse = max(dot(normal, lightDirection), 0.0);
    float grime = 0.82 + 0.18 * sin(v_texCoord.x * 37.0 + v_texCoord.y * 19.0);

    vec3 albedo = u_materialColor * grime;
    vec3 ambient = vec3(0.055, 0.065, 0.075);
    vec3 lit = albedo * (ambient + diffuse * vec3(0.76, 0.68, 0.52));

    float distanceFog = clamp(length(v_worldPosition) / 120.0, 0.0, 1.0);
    vec3 fogColor = vec3(0.11, 0.12, 0.105);
    vec3 color = mix(lit, fogColor, distanceFog * 0.45);

    fragColor = vec4(color, 1.0);
}
