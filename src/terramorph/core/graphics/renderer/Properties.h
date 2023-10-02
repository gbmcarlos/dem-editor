#pragma once

#include "terramorph/pch.h"
#include "terramorph/core/graphics/components/PlanetComponent.h"
#include "terramorph/core/graphics/components/TerrainComponent.h"
#include "terramorph/core/graphics/components/StampComponent.h"

namespace terramorph::Core {

    struct QuadProperties {

        QuadProperties(Quad& quad)
            : m_quadPosition(quad.m_position), m_leftSizeFactor(quad.m_leftSizeRatio), m_bottomSizeFactor(quad.m_bottomSizeRatio), m_rightSizeFactor(quad.m_rightSizeRatio), m_topSizeFactor(quad.m_topSizeRatio) {}

        unsigned int m_quadPosition;
        float m_leftSizeFactor;
        float m_bottomSizeFactor;
        float m_rightSizeFactor;
        float m_topSizeFactor;
        unsigned int m_textureIndex; // For compatibility with the batch renderer
        glm::vec2 m_pad2;

    };

    struct CameraFrustumPlane {
        glm::vec3 m_normal;
        float distance;
    };

    struct CameraFrustum {

        CameraFrustumPlane m_nearPlane;
        CameraFrustumPlane m_farPlane;
        CameraFrustumPlane m_leftPlane;
        CameraFrustumPlane m_rightPlane;
        CameraFrustumPlane m_bottomPlane;
        CameraFrustumPlane m_topPlane;

    };

    struct TerrainProperties {

        TerrainProperties(PlanetComponent& planetComponent, TerrainComponent& terrainComponent) {

            gaunlet::Scene::Frustum originalFrustum = terrainComponent.getCamera()->getFrustum();
            m_cameraFrustum = {
                {originalFrustum.m_nearPlane.m_normal, originalFrustum.m_nearPlane.m_distance},
                {originalFrustum.m_farPlane.m_normal, originalFrustum.m_farPlane.m_distance},
                {originalFrustum.m_leftPlane.m_normal, originalFrustum.m_leftPlane.m_distance},
                {originalFrustum.m_rightPlane.m_normal, originalFrustum.m_rightPlane.m_distance},
                {originalFrustum.m_bottomPlane.m_normal, originalFrustum.m_bottomPlane.m_distance},
                {originalFrustum.m_topPlane.m_normal, originalFrustum.m_topPlane.m_distance},
            };

            m_basisMatrix = planetComponent.getBasisMatrix();
            m_radius = planetComponent.m_radius;
            m_triangleSize = (float) planetComponent.m_triangleSize;
            m_lonStart = -glm::radians(planetComponent.m_coverage);
            m_lonEnd = glm::radians(planetComponent.m_coverage);
            m_latStart = -glm::radians(planetComponent.m_coverage);
            m_latEnd = glm::radians(planetComponent.m_coverage);
            m_lonShift = glm::radians(planetComponent.m_coverage);

            m_terrainWidth = terrainComponent.getMeshWidth();
            m_terrainDepth = terrainComponent.getMeshDepth();
            m_maxHeight = terrainComponent.m_maxHeight;
            m_heightmapResolution = (float) terrainComponent.getHeightmapResolution();

        }

        CameraFrustum m_cameraFrustum;
        glm::mat4 m_basisMatrix;
        float m_radius;
        float m_lonStart;
        float m_lonEnd;
        float m_latStart;
        float m_latEnd;
        float m_lonShift;
        float m_terrainWidth;
        float m_terrainDepth;
        float m_maxHeight;
        float m_triangleSize;
        float m_heightmapResolution;
        int m_entityId;
        glm::vec2 m_stampUvOrigin = {0, 0};
        float m_stampUvWidth = 0;
        float m_stampUvHeight = 0;

    };

}