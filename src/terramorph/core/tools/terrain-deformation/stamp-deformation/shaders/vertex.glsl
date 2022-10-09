#version 410 core

// Vertex attributes
layout (location = 0) in vec4 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_textureCoordinates;
layout (location = 3) in uint a_entityIndex;

// Outputs
out vec2 v_heightmapTextureCoordinates;
out vec2 v_stampTextureCoordinates;
out vec3 v_normal;
flat out uint v_entityIndex;

// Uniforms
uniform vec2 u_stampUVOrigin;
uniform float u_stampUVWidth;
uniform float u_stampUVHeight;

vec2 computeHeightmapTextureCoordinates();

void main() {

    // The textureCoordinates of the heightmap need to be deduced from the stamp position and size, and the heightmap dimensions
    v_heightmapTextureCoordinates = computeHeightmapTextureCoordinates();
    // The textureCoordinates in the vertex atrtibute are those of the quad itself
    v_stampTextureCoordinates = a_textureCoordinates;

    v_normal = a_normal;
    v_entityIndex = a_entityIndex;
    gl_Position = a_position;

}

vec2 computeHeightmapTextureCoordinates() {

    float u = a_textureCoordinates.x == 0 ? (u_stampUVOrigin.x - (u_stampUVWidth/2)) : (u_stampUVOrigin.x + (u_stampUVWidth/2));
    float v = a_textureCoordinates.y == 0 ? (u_stampUVOrigin.y - (u_stampUVHeight/2)) : (u_stampUVOrigin.y + (u_stampUVHeight/2));

    return vec2(u, v);

}