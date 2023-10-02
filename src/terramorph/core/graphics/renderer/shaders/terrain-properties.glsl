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
    mat4 basisMatrix;
    float radius;
    float lonStart;
    float lonEnd;
    float latStart;
    float latEnd;
    float lonShift;
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

vec2 planeUV2lonLat(vec2 planeUV);
vec2 planeUV2sphereUV(vec2 planeUV);
vec4 planeUV2spherePoint(vec2 planeUV);
vec2 sphereUV2lonLat(vec2 sphereUV);
vec4 sphereUV2spherePoint(vec2 sphereUV);
vec2 lonLat2sphereUV(vec2 lonLat);
vec4 lonLat2spherePoint(vec2 lonLat);
vec2 spherePoint2sphereUV(vec4 spherePoint);

vec2 planeUV2LonLat(vec2 planeUV) {

    float lon = lonStart + (planeUV.x * (lonEnd - lonStart));
    float lat = latStart + (planeUV.y * (latEnd - latStart));
    lon += lonShift;

    return vec2(lon, lat);

}

vec2 planeUV2sphereUV(vec2 planeUV) {

    vec4 spherePoint = planeUV2spherePoint(planeUV);
    return spherePoint2sphereUV(spherePoint * basisMatrix);

}

vec4 planeUV2spherePoint(vec2 planeUV) {

    vec2 lonLat = planeUV2LonLat(planeUV);
    return lonLat2spherePoint(lonLat);

}

vec2 sphereUV2lonLat(vec2 sphereUV) {

    float u = sphereUV.x;
    float lon = 1.0f - u;
    lon -= 0.5;
    lon *= radians(180.0f)*2.0f;
    lon += lonShift;

    float v = sphereUV.y;
    float lat = v - 0.5f;
    lat *= radians(180.0f);

    return vec2(lon, lat);

}

vec4 sphereUV2spherePoint(vec2 sphereUV) {

    vec2 lonLat = sphereUV2lonLat(sphereUV);
    return lonLat2spherePoint(lonLat);

}

vec2 lonLat2sphereUV(vec2 lonLat) {

    float lon = lonLat.x;
    float u = lon - lonShift;
    u /= (radians(180.0f)*2.0f);
    u += 0.5f;
    u = 1.0f - u;

    float lat = lonLat.y;
    float v = lat / radians(180.0f);
    v += 0.5f;

    return vec2(u, v);

}

vec4 lonLat2spherePoint(vec2 lonLat) {

    float x = cos(lonLat.x) * cos(lonLat.y);
    float y = sin(lonLat.y);
    float z = sin(lonLat.x) * cos(lonLat.y);

    return vec4(x, y, z, 1);

}

vec2 spherePoint2sphereUV(vec4 spherePoint) {

    float pi = radians(180.0f);

    float u = (atan(spherePoint.x, spherePoint.z) / pi + 1) / 2.0f;
    float v = asin(spherePoint.y) / pi + 0.5;

    return vec2(u, v);

}