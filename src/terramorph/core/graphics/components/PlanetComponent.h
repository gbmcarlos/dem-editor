#pragma once

#include "terramorph/core/compute/quad-tree/QuadTree.h"
#include "gaunlet/graphics/Vertex.h"

namespace terramorph::Core {

    class PlanetComponent {

    public:

        PlanetComponent(float radius, float triangleSize, float targetResolution, float resolutionSlope, float coverage, gaunlet::Core::Ref<gaunlet::Scene::PerspectiveCamera> camera);

        std::vector<Core::Quad> getContent();
        glm::mat3 getBasisMatrix();

        bool quadSubdivisionFunctor(float leftU, float rightU, float bottomU, float topV);
        gaunlet::Graphics::Vertex quadVertexFunctor(float u, float v);

        // Conversion from UV coordinates
        glm::vec2 planeUV2LonLat(float u, float v);
        glm::vec3 planeUV2spherePoint(float u, float v);

        // Conversion from lon-lat
        glm::vec3 lonLat2SpherePoint(float lon, float lat);

        float m_radius;
        float m_triangleSize;
        float m_targetResolution;
        float m_resolutionSlope;
        float m_coverage;

    private:

        gaunlet::Core::Ref<gaunlet::Scene::PerspectiveCamera> m_camera;

        glm::vec3 m_cameraPosition;
        float m_longitudeStart;
        float m_longitudeEnd;
        float m_latitudeStart;
        float m_latitudeEnd;
        float m_longitudeShift;
        glm::mat3 m_basisMatrix;

    };

}