#version 410 core

// Outputs
in vec2 v_heightmapTextureCoordinates;
in vec2 v_stampTextureCoordinates;
in vec3 v_normal;
flat in uint v_entityIndex;

layout (location = 0) out float o_height;

uniform float u_timeStep;
uniform float u_strength;

uniform sampler2D heightmap;
uniform sampler2D stamp;

void main() {

    float originalHeight = texture(heightmap, v_heightmapTextureCoordinates).x;
    float stampHeight = texture(stamp, v_stampTextureCoordinates).x;
    float deformationFactor = (stampHeight - 0.5f) * 2.0f; // In the stamp, black (0) means down and white (1) means up
    deformationFactor = deformationFactor * u_timeStep * u_strength;
    o_height = originalHeight + deformationFactor;

}
