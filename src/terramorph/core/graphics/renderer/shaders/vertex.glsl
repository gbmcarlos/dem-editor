#version 410 core

#include "GL_PREFABS_PATH/shaders/includes/scene-properties.glsl"
#include "entity-properties.glsl"
#include "terrain-properties.glsl"

// Vertex attributes
layout (location = 0) in vec4 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_textureCoordinates;
layout (location = 3) in uint a_entityIndex;

// Outputs
out vec2 v_planeUV;
flat out uint v_entityIndex;

// Textures
uniform sampler2D heightmap;

void main() {

    v_planeUV = a_textureCoordinates;
    v_entityIndex = a_entityIndex;

    vec4 vertexPosition = a_position;
//    vec2 sphereUV = planeUV2sphereUV(v_planeUV);
//    float height = texture(heightmap, sphereUV).x * maxHeight;
//    vertexPosition += vec4(a_normal, 1) * height;

    gl_Position = vertexPosition;

}