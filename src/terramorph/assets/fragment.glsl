#version 410 core

// Outputs
in vec2 v_textureCoordinates;
in vec3 v_normal;
flat in uint v_entityIndex;

layout (location = 0) out vec4 o_color;

uniform vec2 u_brushOrigin;
uniform float u_brushSize;
uniform float u_timeStep;
uniform float u_strength;

uniform sampler2D heightmap;

float getDeformationFactor();

void main() {

    vec4 originalTexel = texture(heightmap, v_textureCoordinates);
    float deformationFactor = getDeformationFactor();
    o_color = originalTexel + (vec4(1, 1, 1, 1) * deformationFactor);

}

float getDeformationFactor() {

    float inverseDistance = 1.0f - distance(u_brushOrigin, v_textureCoordinates) / u_brushSize;
    float circleFactor = smoothstep(0.0f, 1.0f, inverseDistance);

    return circleFactor * u_strength * u_timeStep;

}
