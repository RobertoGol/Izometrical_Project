#version 330 core

in vec2 v_texCoord;

uniform sampler2D u_sceneTexture;
uniform vec2 u_resolution;

out vec4 fragColor;

void main()
{
    vec3 color = texture(u_sceneTexture, v_texCoord).rgb;
    vec2 centered = v_texCoord - vec2(0.5);
    float vignette = 1.0 - dot(centered, centered) * 0.22;
    vec3 graded = pow(color, vec3(0.96)) * vignette;
    fragColor = vec4(graded, 1.0);
}
