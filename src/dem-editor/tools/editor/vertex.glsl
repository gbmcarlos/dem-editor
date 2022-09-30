#version 410 core

// Vertex attributes
layout (location = 0) in vec4 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_textureCoordinates;
layout (location = 3) in uint a_entityIndex;

// Outputs
out vec2 v_textureCoordinates;
out vec3 v_normal;
flat out uint v_entityIndex;

void main() {

    v_textureCoordinates = a_textureCoordinates;
    v_normal = a_normal;
    v_entityIndex = a_entityIndex;
    gl_Position = a_position;

}