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
    float triangleSize;
    float maxHeight;
    vec2 stampOrigin;
    float stampSize;
    int entityId;
    float terrainSize;
    float heightmapSize;
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

vec4 getDirectionalLightColor(
    vec3 color, vec3 direction,
    float ambientIntensity, float diffuseIntensity,
    vec3 normal
);

void main() {

    vec4 textureColor = texture(heightmap, te_textureCoordinates);
    vec4 directionalLightColor = getDirectionalLightColor(directionalLight.color, directionalLight.direction, directionalLight.ambientIntensity, directionalLight.diffuseIntensity, te_normal);
    float halfSize = stampSize/2;

    o_color = vec4(0.2f, 0.5f, 0.8f, 1.0f) * directionalLightColor;
    float stampLeft = (stampOrigin.x - halfSize);
    float stampRight = (stampOrigin.x + halfSize);
    float stampBottom = (stampOrigin.y - halfSize);
    float stampTop = (stampOrigin.y + halfSize);

    if (stampSize > 0.0f &&
        te_textureCoordinates.x > stampLeft
        && te_textureCoordinates.x < stampRight
        && te_textureCoordinates.y > stampBottom
        && te_textureCoordinates.y < stampTop
    ) {
        vec2 stampTextureCoordinates = vec2(
            (te_textureCoordinates.x - stampLeft) / stampSize,
            (te_textureCoordinates.y - stampBottom) / stampSize
        );
        vec4 stampColor = texture(brushStamp, stampTextureCoordinates);
        o_color += stampColor;
    }

    o_entityId = entityId;
    o_terrainPosition = vec3(te_textureCoordinates.x, textureColor.y, te_textureCoordinates.y);

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
