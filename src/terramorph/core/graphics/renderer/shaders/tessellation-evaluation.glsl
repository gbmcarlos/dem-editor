#version 410 core

#include "GL_PREFABS_PATH/shaders/includes/scene-properties.glsl"
#include "entity-properties.glsl"
#include "terrain-properties.glsl"

layout (quads, equal_spacing, ccw) in;

in vec2 tc_planeUV[];

out vec2 te_planeUV;
out vec3 te_normal;
out vec3 te_vertexColor;

// Textures
uniform sampler2D heightmap;

vec2 interpolatePlaneUV();
vec3 computeNormal(vec2 sphereUV);
vec3 getVertexColor();
vec2 interpolate2(vec2 point, vec2 vector1, vec2 vector2, vec2 vector3, vec2 vector4);
vec4 interpolate4(vec2 point, vec4 vector1, vec4 vector2, vec4 vector3, vec4 vector4);

void main() {

    vec2 planeUV = interpolatePlaneUV();
    te_vertexColor = getVertexColor();

    vec4 spherePoint = planeUV2spherePoint(planeUV);
    vec4 vertexPosition = spherePoint * radius * basisMatrix;

    vec2 sphereUV = planeUV2sphereUV(planeUV);
    float height = texture(heightmap, sphereUV).x * maxHeight;
    vertexPosition += normalize(vertexPosition) * height;

    te_normal = computeNormal(sphereUV);

    te_planeUV = planeUV;
    gl_Position = projection * view * vec4(vertexPosition.xyz, 1);

}

vec2 interpolatePlaneUV() {
    return interpolate2(gl_TessCoord.xy, tc_planeUV[0], tc_planeUV[1], tc_planeUV[2], tc_planeUV[3]);
}

vec3 computeNormal(vec2 sphereUV) {

    float texelWidth = 1.0f / (terrainWidth * heightmapResolution);
    float texelHeight = 1.0f / (terrainDepth * heightmapResolution);

    vec4 heights;
    heights.x = texture(heightmap, sphereUV + vec2(0, -1.0f * texelHeight)).x * maxHeight; // down
    heights.y = texture(heightmap, sphereUV + vec2(-1.0f * texelWidth, 0)).x * maxHeight; // left
    heights.z = texture(heightmap, sphereUV + vec2(1.0f * texelWidth, 0)).x * maxHeight; // right
    heights.w = texture(heightmap, sphereUV + vec2(0, 1.0f * texelHeight)).x * maxHeight; // up

    vec3 normal;
    normal.z = heights.w - heights.z; // up - down, vertical
    normal.x = heights.z - heights.y; // right - left, horizontal
    normal.y = texelWidth * terrainWidth; // pixel space -> uv space -> world space

    return normalize(normal);

}

vec3 getVertexColor() {

    float edgeThreshold = 0.001f;

    if (gl_TessCoord.x < edgeThreshold || gl_TessCoord.x > 1-edgeThreshold || gl_TessCoord.y < edgeThreshold || gl_TessCoord.y > 1-edgeThreshold) {
        return vec3(1, 0, 0);
    } else {
        return vec3(0, 1, 0);
    }

}

vec2 interpolate2(vec2 point, vec2 vector1, vec2 vector2, vec2 vector3, vec2 vector4) {

    vec2 a = mix(vector1, vector2, point.x);
    vec2 b = mix(vector4, vector3, point.x);
    vec2 c = mix(a, b, point.y);

    return c.xy;

}

vec4 interpolate4(vec2 point, vec4 vector1, vec4 vector2, vec4 vector3, vec4 vector4) {

    vec4 a = mix(vector1, vector2, point.x);
    vec4 b = mix(vector4, vector3, point.x);
    vec4 c = mix(a, b, point.y);

    return c.xyzw;

}