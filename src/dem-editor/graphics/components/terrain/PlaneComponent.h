#pragma once

#include "gaunlet/scene/camera/PerspectiveCamera.h"
#include "dem-editor/graphics/components/procedural-plane/ProceduralPlane.h"

namespace DemEditor {

    struct PlaneComponent {

        PlaneComponent(float size, float targetResolution, float resolutionSlope, float triangleSize, float maxHeight, gaunlet::Core::Ref<gaunlet::Scene::PerspectiveCamera> camera)
            : m_size(size), m_targetResolution(targetResolution), m_resolutionSlope(resolutionSlope), m_triangleSize(std::max(triangleSize, 4.0f)), m_maxHeight(maxHeight), m_camera(std::move(camera)) {
        }

        std::vector<PlaneQuad> getContent() {
            return QuadTreePatch::compute(
                m_size,
                m_targetResolution,
                std::max(m_resolutionSlope, 0.1f),
                m_camera->getPosition(),
                m_camera->getFrustum()
            );
        }

        float m_size;
        float m_targetResolution;
        float m_resolutionSlope;
        float m_triangleSize;
        float m_maxHeight;
        gaunlet::Core::Ref<gaunlet::Scene::PerspectiveCamera> m_camera;

    };

}