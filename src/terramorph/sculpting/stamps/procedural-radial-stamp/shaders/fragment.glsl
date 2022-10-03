#version 410 core

// Outputs
in vec2 v_textureCoordinates;
in vec3 v_normal;
flat in uint v_entityIndex;

layout (location = 0) out vec3 o_grayscale;

uniform float minValue;
uniform float maxValue;
uniform float stepMin;
uniform float stepMax;

void main() {

    float inverseDistance = 1.0f - distance(vec2(0.5f, 0.5f), v_textureCoordinates);
    float factor = smoothstep(stepMin, stepMax, inverseDistance);
    factor = max(factor, minValue);
    factor = min(factor, maxValue);

    o_grayscale =  vec3(1, 1, 1) * factor;

}

