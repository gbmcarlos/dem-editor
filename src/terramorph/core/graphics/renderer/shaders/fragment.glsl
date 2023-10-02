#version 410 core

#include "GL_PREFABS_PATH/shaders/includes/scene-properties.glsl"
#include "entity-properties.glsl"
#include "terrain-properties.glsl"

// Inputs
in vec2 te_planeUV;
in vec3 te_normal;
in vec3 te_vertexColor;
flat in uint te_entityIndex;

// Outputs
layout (location = 0) out vec4 o_color;
layout (location = 1) out int o_entityId;
layout (location = 2) out vec2 o_planetPosition;

// Textures
uniform sampler2D heightmap;
uniform sampler2D brushStamp;

bool withinStamp(vec2 sphereUV);
vec2 getStampUV(vec2 sphereUV);
vec4 getStampCornersUV();

vec4 getDirectionalLightColor(
    vec3 color, vec3 direction,
    float ambientIntensity, float diffuseIntensity,
    vec3 normal
);

void main() {

    vec2 sphereUV = planeUV2sphereUV(te_planeUV);
    float height = texture(heightmap, sphereUV).x;
    vec4 directionalLightColor = getDirectionalLightColor(directionalLight.color, directionalLight.direction, directionalLight.ambientIntensity, directionalLight.diffuseIntensity, te_normal);

    o_color = vec4(0.2f, 0.5f, 0.8f, 1.0f) * directionalLightColor;

    if (withinStamp(sphereUV)) {
//        vec4 stampColor = texture(brushStamp, getStampUV(sphereUV));
        vec4 stampColor = vec4(1, 0, 0, 1);
        o_color = stampColor;
    }

//    o_color = vec4(te_vertexColor, 1);

    o_entityId = entityId;
    o_planetPosition = sphereUV;

}

bool withinStamp(vec2 sphereUV) {

    vec4 stampPoint = sphereUV2spherePoint(stampUvOrigin) * radius;
    vec3 planeNormal = normalize(stampPoint.xyz);
    // R - h = Rsinb
    // R - h = Rsin(90-b)
    float angle = 30.0f;
    float planeDistance = radius * sin(radians(90.0f - angle));
    vec4 localPoint = planeUV2spherePoint(te_planeUV) * radius * basisMatrix;

    float distance = dot(planeNormal, vec3(localPoint)) - planeDistance;
    return distance > 0;

    float stampLeft = (stampUvOrigin.x - (stampUvWidth/2));
    float stampRight = (stampUvOrigin.x + (stampUvWidth/2));

    // By default, a pixel will be inside the stamp if it's between its left and right edges
    bool withinHorizontal = (stampLeft < sphereUV.x) && (sphereUV.x < stampRight);
    // If the stamp extends beyond the left edge (x<0 || x>1), then a pixel is inside if 1) it's between the left edge and 1 or 2) it's between 0 and the right edge
    if (stampLeft < 0.0f ||  stampRight > 1.0f) {
        float normalizedStampLeft = stampLeft < 0.0f ? 1.0f + stampLeft : stampLeft;
        float normalizedStampRight = stampRight > 1.0f ? stampRight - 1.0f : stampRight;
        withinHorizontal = (normalizedStampLeft < sphereUV.x && sphereUV.x < 1.0f) || (0.0f < sphereUV.x && sphereUV.x < normalizedStampRight);
    }

    float stampBottom = (stampUvOrigin.y - (stampUvHeight/2));
    float stampTop = (stampUvOrigin.y + (stampUvHeight/2));

    return stampUvWidth > 0.0f && stampUvHeight > 0.0f
        && withinHorizontal
        && stampBottom < sphereUV.y
        && sphereUV.y < stampTop;

}

vec2 getStampUV(vec2 sphereUV) {

    float stampLeft = (stampUvOrigin.x - (stampUvWidth/2));
    if (stampLeft < 0.0f) {
        stampLeft = 1.0f - abs(stampLeft);
    }
    float stampBottom = (stampUvOrigin.y - (stampUvHeight/2));

    return vec2(
        (sphereUV.x - stampLeft) / stampUvWidth,
        (sphereUV.y - stampBottom) / stampUvHeight
    );

}

vec4 getStampCornersUV() {

    float stampLeft = (stampUvOrigin.x - (stampUvWidth/2));
    if (stampLeft < 0.0f) {
        stampLeft = 1.0f + stampLeft;
    }
    float stampRight = (stampUvOrigin.x + (stampUvWidth/2));
    if (stampRight > 1.0f) {
        stampRight = stampRight - 1.0f;
    }
    float stampBottom = (stampUvOrigin.y - (stampUvHeight/2));
    if (stampBottom < 0.0f) {
        stampBottom = 1.0f + stampBottom;
    }
    float stampTop = (stampUvOrigin.y + (stampUvHeight/2));
    if (stampTop > 1.0f) {
        stampTop = stampTop - 1.0f;
    }

    return vec4(stampLeft, stampRight, stampBottom, stampTop);

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