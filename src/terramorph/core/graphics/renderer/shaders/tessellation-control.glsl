#version 410 core

#include "GL_PREFABS_PATH/shaders/includes/scene-properties.glsl"
#include "entity-properties.glsl"
#include "terrain-properties.glsl"

layout (vertices = 4) out;

in vec2 v_planeUV[];
flat in uint v_entityIndex[];

out vec2 tc_planeUV[];

// Textures
uniform sampler2D heightmap;

const uint LEFT_BOTTOM_POSITION = 0;
const uint RIGHT_BOTTOM_POSITION = 1;
const uint RIGHT_TOP_POSITION = 2;
const uint LEFT_TOP_POSITION = 3;

const uint LEFT_EDGE = 0;
const uint BOTTOM_EDGE = 1;
const uint RIGHT_EDGE = 2;
const uint TOP_EDGE = 3;

bool withinFrustum(vec4 corner0, vec4 corner1, vec4 corner2, vec4 corner3);
bool quadOver(FrustumPlane plane, vec4 corner0, vec4 corner1, vec4 corner2, vec4 corner3);
bool pointOver(FrustumPlane plane, vec4 point);

int getTessellationLevel(uint edgeIndex, uint position, float sizeFactor, float triangleSize);

int getLargerEdgeTessellationLevel(uint edgeIndex, uint position, float triangleSize);
vec4 getExtendedEdgeVertex(vec2 fixedPlaneUV, vec2 planeUVtoExtend);
float getEdgeTessellationFactor(vec4 edgeStart, vec4 edgeEnd, float triangleSize);

int roundUp(float value);
int roundUpEven(float value);

vec2 getVertexIndices(uint edgeIndex);

void main() {

    tc_planeUV[gl_InvocationID] = v_planeUV[gl_InvocationID];
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    if (gl_InvocationID == 0) {

        vec4 vertexPosition0 = gl_in[0].gl_Position;
        vec4 vertexPosition1 = gl_in[1].gl_Position;
        vec4 vertexPosition2 = gl_in[2].gl_Position;
        vec4 vertexPosition3 = gl_in[3].gl_Position;

        if (!withinFrustum(vertexPosition0 * basisMatrix, vertexPosition1 * basisMatrix, vertexPosition2 * basisMatrix, vertexPosition3 * basisMatrix)) {
            gl_TessLevelOuter[0] = -1;
            gl_TessLevelOuter[1] = -1;
            gl_TessLevelOuter[2] = -1;
            gl_TessLevelOuter[3] = -1;

            gl_TessLevelInner[0] = -1;
            gl_TessLevelInner[1] = -1;

        } else {

            uint entityIndex = v_entityIndex[gl_InvocationID];
            EntityPropertySet entityProperties = entityPropertySets[entityIndex];

            // Left edge, from 0 to 3, so pointing up
            float tessellationLevelLeft = getTessellationLevel(LEFT_EDGE, entityProperties.position, entityProperties.leftSizeFactor, triangleSize);
            // Bottom edge, from 0 to 1, so pointing right
            float tessellationLevelBottom = getTessellationLevel(BOTTOM_EDGE, entityProperties.position, entityProperties.bottomSizeFactor, triangleSize);
            // Right edge, from 1 to 2, so pointing up
            float tessellationLevelRight = getTessellationLevel(RIGHT_EDGE, entityProperties.position, entityProperties.rightSizeFactor, triangleSize);
            // Top edge, from 3 to 2, so pointing right
            float tessellationLevelTop = getTessellationLevel(TOP_EDGE, entityProperties.position, entityProperties.topSizeFactor, triangleSize);

            float tessellationLevelVertical = max(tessellationLevelBottom, tessellationLevelTop);
            float tessellationLevelHorizontal = max(tessellationLevelLeft, tessellationLevelRight);

            gl_TessLevelOuter[0] = tessellationLevelLeft;
            gl_TessLevelOuter[1] = tessellationLevelBottom;
            gl_TessLevelOuter[2] = tessellationLevelRight;
            gl_TessLevelOuter[3] = tessellationLevelTop;

            gl_TessLevelInner[0] = tessellationLevelVertical;
            gl_TessLevelInner[1] = tessellationLevelHorizontal;

        }

    }

}

int getTessellationLevel(uint edgeIndex, uint position, float sizeFactor, float triangleSize) {

    if (triangleSize < 1) {
        return 1;
    }

    vec2 edgeVertexIndices = getVertexIndices(edgeIndex);
    vec4 edgeStart = gl_in[int(edgeVertexIndices.x)].gl_Position;
    vec4 edgeEnd = gl_in[int(edgeVertexIndices.y)].gl_Position;

    float edgeTessellationFactor = getEdgeTessellationFactor(edgeStart, edgeEnd, triangleSize);

    // A size factor of 0 means that there is no neighbour
    // So we can leave it as is
    if (sizeFactor == 0.0f) {
        return int(roundUpEven(edgeTessellationFactor));
    }

    // A size factor of 1.0f means that this quad and the neighbour are the same size;
    // So we can leave it as is
    if (sizeFactor == 1.0f) {
        return int(roundUpEven(edgeTessellationFactor));
    }

    // A size factor of 0.5f means that the neighbour is smaller, and this quad is bigger
    // The smaller quad will be adapted to the big one, so we need to make sure the big one's is even
    if (sizeFactor == 0.5f) {
        int roundedTessellationLevel = roundUpEven(edgeTessellationFactor);
        return roundedTessellationLevel;
    }

    // A size factor of 2.0f means that the neigbour is bigger, and this quad is smaller
    // So we need to simulate the neighbour's edge projected length, and use half of that
    if (sizeFactor == 2.0f) {
        int largerEdgeTessellationLevel = getLargerEdgeTessellationLevel(edgeIndex, position, triangleSize);
        int adaptedTessellationLevel = int(largerEdgeTessellationLevel / 2);
        return adaptedTessellationLevel;
    }

    return 1;

}

// Assume that the vertical edges always point up, and the horizontal edges always point right
int getLargerEdgeTessellationLevel(uint edgeIndex, uint position, float triangleSize) {

    bool extendForward;

    // If we need a vertical edge, check whether we are at the bottom or at the top
    if (edgeIndex == LEFT_EDGE || edgeIndex == RIGHT_EDGE) {
        extendForward = (position == LEFT_BOTTOM_POSITION || position == RIGHT_BOTTOM_POSITION); // We extend forward (up), if we are at the bottom. Otherwise, extend down
    } else { // If we need a horizontal edge, check whether we are at the left or at the right
        extendForward = (position == LEFT_BOTTOM_POSITION || position == LEFT_TOP_POSITION); // We extend forward (right), if we are at the left. Otherwise, extend right
    }

    // Now we extend our edge, either forward or backward, to simulate the larger neighbour's edge
    float edgeTessellationFactor;
    vec2 edgeVertexIndices = getVertexIndices(edgeIndex);
    vec4 edgeStart = gl_in[int(edgeVertexIndices.x)].gl_Position;
    vec4 edgeEnd = gl_in[int(edgeVertexIndices.y)].gl_Position;
    vec2 edgeStartUV = v_planeUV[int(edgeVertexIndices.x)];
    vec2 edgeEndUV = v_planeUV[int(edgeVertexIndices.y)];

    if (extendForward) {
        vec4 extendedEdgeEnd = getExtendedEdgeVertex(edgeStartUV, edgeEndUV);
        edgeTessellationFactor = getEdgeTessellationFactor(edgeStart, extendedEdgeEnd, triangleSize);
    } else {
        vec4 extendedEdgeStart = getExtendedEdgeVertex(edgeEndUV, edgeStartUV);
        edgeTessellationFactor = getEdgeTessellationFactor(extendedEdgeStart, edgeEnd, triangleSize);
    }

    int roundedTessellationLevel = int(roundUpEven(edgeTessellationFactor));

    // Make sure that we don't go below 4 (so a smaller neighbour won't go below 2), or above 64 (hard limit)
    int clampedTessellationLevel = int(max(roundedTessellationLevel, 4));
    clampedTessellationLevel = int(min(roundedTessellationLevel, 64));

    return clampedTessellationLevel;

}

vec4 getExtendedEdgeVertex(vec2 fixedPlaneUV, vec2 planeUVtoExtend) {

    vec2 extendedPlaneUV = planeUVtoExtend + (planeUVtoExtend - fixedPlaneUV);
    vec4 spherePoint = planeUV2spherePoint(extendedPlaneUV);
    vec4 extendedVertex = spherePoint * radius;

//    vec2 sphereUV = planeUV2sphereUV(extendedPlaneUV);
//    float height = texture(heightmap, sphereUV).x * maxHeight;
//    extendedVertex += normalize(spherePoint) * height;

    return extendedVertex;

}

// https://xeolabs.com/pdfs/OpenGLInsights.pdf
float getEdgeTessellationFactor(vec4 edgeStart, vec4 edgeEnd, float triangleSize) {

    vec2 viewportDimensions = vec2(viewport.z - viewport.x, viewport.w - viewport.y);
    vec4 viewCenter = vec4(0, 0, 0, 1);

    vec4 p1 = (edgeStart + edgeEnd) / 2;
    vec4 p2 = viewCenter;
    p2.y += distance(edgeStart, edgeEnd);

    p1 = projection * view * p1;
    p2 = projection * view * p2;
    p1 /= p1.w;
    p2 /= p2.w;

    float l = length((p1.xy - p2.xy) * viewportDimensions * 0.5);
    return clamp(l / triangleSize, 1, 64);

}

// https://gitee.com/T_D/Vulkan/blob/master/data/shaders/terraintessellation/terrain.tesc
float __getEdgeTessellationFactor(vec4 edgeStart, vec4 edgeEnd, float triangleSize) {

    // Calculate edge mid point
    vec4 midPoint = 0.5 * (edgeStart + edgeEnd);
    // Sphere radius as distance between the control points
    float radius = distance(edgeStart, edgeEnd) / 2.0;

    // View space
    vec4 v0 = view * midPoint;

    // Project into clip space
    vec4 clip0 = (projection * (v0 - vec4(radius, vec3(0.0))));
    vec4 clip1 = (projection * (v0 + vec4(radius, vec3(0.0))));

    // Get normalized device coordinates
    clip0 /= clip0.w;
    clip1 /= clip1.w;

    // Convert to viewport coordinates
    vec2 viewportDimensions = vec2(viewport.z - viewport.x, viewport.w - viewport.y);
    clip0.xy *= viewportDimensions;
    clip1.xy *= viewportDimensions;

    // Return the tessellation factor based on the screen size
    // given by the distance of the two edge control points in screen space
    // and a reference (min.) tessellation size for the edge set by the application
    return clamp(distance(clip0, clip1) / triangleSize, 1.0, 64.0);

}

// https://github.com/GPUOpen-LibrariesAndSDKs/Tessellation/blob/master/sample/data/tess.cont
float _getEdgeTessellationFactor(vec4 edgeStart, vec4 edgeEnd, float triangleSize) {

    vec3 sphereCenter = (edgeStart.xyz + edgeEnd.xyz) * 0.5f;
    vec4 viewSphereCenter = view * vec4(sphereCenter, 1.0);
    vec4 viewOrigin = view * vec4(0, 0, 0, 1.0);
    float cameraDistFromSphereCenter = length(viewSphereCenter.xyz);
    float cameraDistFromOrigin = length(viewOrigin.xyz);

    vec4 clipEdgeStart = projection * view * vec4(edgeStart.xyz, 1.0);
    vec4 clipEdgeEnd = projection * view * vec4(edgeEnd.xyz, 1.0);
    float clipSphereDiameter = distance(clipEdgeStart, clipEdgeEnd);

    float viewportHeight = viewport.w - viewport.y;
    float screenSphereDiameter = clipSphereDiameter * viewportHeight;

    float factor = (screenSphereDiameter) / triangleSize;
    return factor;

}

int roundUp(float value) {

    int floored = int(floor(value));
    int result = (float(floored) == value) ? floored : floored + 1;
    return result;

}

int roundUpEven(float value) {

    int rounded = roundUp(value);
    int result = (rounded % 2 == 0) ? rounded : rounded + 1;
    return result;

}

vec2 getVertexIndices(uint edgeIndex) {

    if (edgeIndex == LEFT_EDGE) {
        return vec2(LEFT_BOTTOM_POSITION, LEFT_TOP_POSITION); // Bottom to top, so it goes up
    } else if (edgeIndex == BOTTOM_EDGE) {
        return vec2(LEFT_BOTTOM_POSITION, RIGHT_BOTTOM_POSITION); // Left to right, so it goes right
    } else if (edgeIndex == RIGHT_EDGE) {
        return vec2(RIGHT_BOTTOM_POSITION, RIGHT_TOP_POSITION); // Bottom to top, so it goes up
    } else { // Top edge
        return vec2(LEFT_TOP_POSITION, RIGHT_TOP_POSITION); // Left to right, so it goes right
    }

}

bool withinFrustum(vec4 corner0, vec4 corner1, vec4 corner2, vec4 corner3) {

    if (!quadOver(cameraFrustum.nearPlane, corner0, corner1, corner2, corner3)) {
        return false;
    }

    if (!quadOver(cameraFrustum.farPlane, corner0, corner1, corner2, corner3)) {
        return false;
    }

    if (!quadOver(cameraFrustum.leftPlane, corner0, corner1, corner2, corner3)) {
        return false;
    }

    if (!quadOver(cameraFrustum.rightPlane, corner0, corner1, corner2, corner3)) {
        return false;
    }

    if (!quadOver(cameraFrustum.bottomPlane, corner0, corner1, corner2, corner3)) {
        return false;
    }

    if (!quadOver(cameraFrustum.topPlane, corner0, corner1, corner2, corner3)) {
        return false;
    }

    return true;

}

bool quadOver(FrustumPlane plane, vec4 corner0, vec4 corner1, vec4 corner2, vec4 corner3) {

    if (pointOver(plane, corner0)) {
        return true;
    }

    if (pointOver(plane, corner1)) {
        return true;
    }

    if (pointOver(plane, corner2)) {
        return true;
    }

    if (pointOver(plane, corner3)) {
        return true;
    }

    return false;

}

bool pointOver(FrustumPlane plane, vec4 point) {

    float distance = dot(plane.normal, vec3(point)) - plane.distance;
    return distance > 0;

}
