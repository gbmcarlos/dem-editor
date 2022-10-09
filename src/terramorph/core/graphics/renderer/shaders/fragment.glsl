#version 410 core

struct EntityPropertySet {
    uint position;
    float leftSizeFactor;
    float bottomSizeFactor;
    float rightSizeFactor;
    float topSizeFactor;
    uint textureIndex;
};

struct DirectionalLight {
    vec3 color;
    float ambientIntensity;
    vec3 direction;
    float diffuseIntensity;
};

// Uniforms
layout (std140) uniform EntityPropertySets {
    EntityPropertySet entityPropertySets[100];
};

layout (std140) uniform SceneProperties {
    mat4 view;
    mat4 projection;
    vec4 viewport;
    DirectionalLight directionalLight;
};

struct FrustumPlane {
    vec3 normal;
    float distance;
};

struct CameraFrustum {
    FrustumPlane nearPlane;
    FrustumPlane farPlane;
    FrustumPlane leftPlane;
    FrustumPlane rightPlane;
    FrustumPlane bottomPlane;
    FrustumPlane topPlane;
};

layout (std140) uniform TerrainProperties {
    CameraFrustum cameraFrustum;
    float terrainWidth;
    float terrainDepth;
    float maxHeight;
    float triangleSize;
    float heightmapResolution;
    int entityId;
    vec2 stampUvOrigin;
    float stampUvWidth;
    float stampUvHeight;
};

// Inputs
in vec2 te_textureCoordinates;
in vec3 te_normal;
flat in uint te_entityIndex;

// Outputs
layout (location = 0) out vec4 o_color;
layout (location = 1) out int o_entityId;
layout (location = 2) out vec3 o_terrainPosition;

// Textures
uniform sampler2D heightmap;
uniform sampler2D brushStamp;

bool withinStamp();
vec2 getStampTextureCoordinates();

vec4 getDirectionalLightColor(
    vec3 color, vec3 direction,
    float ambientIntensity, float diffuseIntensity,
    vec3 normal
);

void main() {

    float height = texture(heightmap, te_textureCoordinates).x;
    vec4 directionalLightColor = getDirectionalLightColor(directionalLight.color, directionalLight.direction, directionalLight.ambientIntensity, directionalLight.diffuseIntensity, te_normal);

    o_color = vec4(0.2f, 0.5f, 0.8f, 1.0f) * directionalLightColor;

    if (withinStamp()) {
        vec4 stampColor = texture(brushStamp, getStampTextureCoordinates());
        o_color += stampColor;
    }

    o_entityId = entityId;
    o_terrainPosition = vec3(te_textureCoordinates.x, height, te_textureCoordinates.y);

}

bool withinStamp() {

    float stampLeft = (stampUvOrigin.x - (stampUvWidth/2));
    float stampRight = (stampUvOrigin.x + (stampUvWidth/2));
    float stampBottom = (stampUvOrigin.y - (stampUvHeight/2));
    float stampTop = (stampUvOrigin.y + (stampUvHeight/2));

    return stampUvWidth > 0.0f && stampUvHeight > 0.0f
        && te_textureCoordinates.x > stampLeft
        && te_textureCoordinates.x < stampRight
        && te_textureCoordinates.y > stampBottom
        && te_textureCoordinates.y < stampTop;

}

vec2 getStampTextureCoordinates() {

    float stampLeft = (stampUvOrigin.x - (stampUvWidth/2));
    float stampBottom = (stampUvOrigin.y - (stampUvHeight/2));

    return vec2(
        (te_textureCoordinates.x - stampLeft) / stampUvWidth,
        (te_textureCoordinates.y - stampBottom) / stampUvHeight
    );

}

vec4 getDirectionalLightColor(vec3 color, vec3 direction, float ambientIntensity, float diffuseIntensity, vec3 normal) {

    vec4 ambientColor = vec4(color * ambientIntensity, 1.0f);
    float diffuseFactor = dot(normalize(normal), -direction);

    if (diffuseFactor > 0) {
        return vec4(color * diffuseIntensity * diffuseFactor, 1.0f) + ambientColor;
    } else {
        return ambientColor;
    }

}
