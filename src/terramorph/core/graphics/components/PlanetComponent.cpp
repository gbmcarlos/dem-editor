#include "PlanetComponent.h"

#include <utility>

#include "terramorph/core/compute/quad-tree/QuadTree.h"

namespace terramorph::Core {

    PlanetComponent::PlanetComponent(float radius, float triangleSize, float targetResolution, float resolutionSlope, float coverage, gaunlet::Core::Ref<gaunlet::Scene::PerspectiveCamera> camera)
        : m_radius(radius), m_triangleSize(triangleSize), m_targetResolution(targetResolution), m_resolutionSlope(resolutionSlope), m_coverage(coverage), m_camera(std::move(camera)) {
        m_longitudeShift = glm::radians(90.0f);
    }

    std::vector<Core::Quad> PlanetComponent::getContent() {

        m_cameraPosition = glm::normalize(m_camera->getPosition()) * m_radius;

        float coverage = glm::radians(m_coverage);
        m_longitudeStart = -coverage;
        m_longitudeEnd = coverage;
        m_latitudeStart = -coverage;
        m_latitudeEnd = coverage;

        glm::vec3 longitudeAxis = m_camera->getRight();
        glm::vec3 latitudeAxis = m_camera->getUp();
//        glm::vec3 longitudeAxis = glm::normalize(glm::cross(glm::vec3(0, 1, 0), m_cameraPosition));
//        glm::vec3 latitudeAxis = glm::normalize(glm::cross(longitudeAxis, m_cameraPosition));

        m_basisMatrix = glm::transpose(glm::mat3(longitudeAxis, latitudeAxis, glm::cross(longitudeAxis, latitudeAxis)));

        return QuadTreePatch::compute(
            std::bind(&PlanetComponent::quadSubdivisionFunctor, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
            std::bind(&PlanetComponent::quadVertexFunctor, this, std::placeholders::_1, std::placeholders::_2)
        );

    }

    glm::mat3 PlanetComponent::getBasisMatrix() {

        glm::vec3 longitudeAxis = m_camera->getRight();
        glm::vec3 latitudeAxis = m_camera->getUp();

        return glm::transpose(glm::mat3(longitudeAxis, latitudeAxis, glm::cross(longitudeAxis, latitudeAxis)));

    }

    bool PlanetComponent::quadSubdivisionFunctor(float leftU, float rightU, float bottomU, float topV) {

        glm::vec3 projectedCameraPosition = m_cameraPosition;

        auto leftBottomCorner = planeUV2spherePoint(leftU, bottomU) * m_radius;
        auto rightBottomCorner = planeUV2spherePoint(rightU, bottomU) * m_radius;
        auto rightTopCorner = planeUV2spherePoint(rightU, topV) * m_radius;
        auto leftTopCorner = planeUV2spherePoint(leftU, topV) * m_radius;

        // The distance between the camera and each corner of the quad
        float distanceLeftBottom = glm::distance(projectedCameraPosition, leftBottomCorner);
        float distanceRightBottom = glm::distance(projectedCameraPosition, rightBottomCorner);
        float distanceRightTop = glm::distance(projectedCameraPosition, rightTopCorner);
        float distanceLeftTop = glm::distance(projectedCameraPosition, leftTopCorner);

        // The distance to the closest corner
        float distance = std::min(std::min(distanceLeftBottom, distanceRightBottom), std::min(distanceRightTop, distanceLeftTop));

        // Calculate the required resolution based on the distance to the camera
        float relativeResolution = (distance * m_resolutionSlope) + 1;
        relativeResolution = std::max(relativeResolution, m_targetResolution);

        float quadWidth = glm::distance(rightBottomCorner, leftBottomCorner);
        float quadHeight = glm::distance(leftTopCorner, leftBottomCorner);

        return quadWidth > relativeResolution || quadHeight > relativeResolution;

    }

    gaunlet::Graphics::Vertex PlanetComponent::quadVertexFunctor(float u, float v) {

        // [0, 1]
        u = (u + 1.0f) / 2.0f;
        v = (v + 1.0f) / 2.0f;

        auto spherePoint = planeUV2spherePoint(u, v);

        return {
            glm::vec4(spherePoint, 1) * m_radius,
            spherePoint,
            {u, v}
        };

    }

    glm::vec2 PlanetComponent::planeUV2LonLat(float u, float v) {

        float lon = m_longitudeStart + (u * (m_longitudeEnd - m_longitudeStart));
        float lat = m_latitudeStart + (v * (m_latitudeEnd - m_latitudeStart));
        lon += m_longitudeShift;

        return {lon, lat};

    }

    glm::vec3 PlanetComponent::planeUV2spherePoint(float u, float v) {

        glm::vec2 lonLat = planeUV2LonLat(u, v);
        return lonLat2SpherePoint(lonLat.x, lonLat.y);

    }

    glm::vec3 PlanetComponent::lonLat2SpherePoint(float lon, float lat) {

        float x = glm::cos(lon) * glm::cos(lat);
        float y = glm::sin(lat);
        float z = glm::sin(lon) * glm::cos(lat);

        return {x, y, z};

    }

}