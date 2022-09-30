#version 410 core

// Outputs
in vec2 v_textureCoordinates;
in vec3 v_normal;
flat in uint v_entityIndex;

layout (location = 0) out vec4 o_color;

uniform sampler2D heightmap;

void main() {

    vec4 originalTexel = texture(heightmap, v_textureCoordinates);
    o_color = originalTexel * 1.1f;

}