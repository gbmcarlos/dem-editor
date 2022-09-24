#pragma once

#include "gaunlet/scene/renderer/ObjectRenderer.h"
#include "gaunlet/prefab/object-renderers/model-renderer/ModelRenderer.h"
#include "gaunlet/graphics/render-pass/SimpleRenderPass.h"
#include "dem-editor/graphics/components/TerrainComponents.h"
#include "gaunlet/scene/camera/Camera.h"

#include "gaunlet/pch.h"

namespace DemEditor {

    struct PlaneQuadEntityProperties {

        PlaneQuadEntityProperties() = default;

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

    class TerrainRenderer {

    public:

        TerrainRenderer(unsigned int propertySetsUniformBufferBindingPoint, unsigned int cameraFrustumUniformBufferBindingPoint)
            : m_renderer({100000, 600000, 10, 1000}) {

            // Create a uniform buffer that will contain the properties of every object, and will be linked to the shader
            m_propertySetsUniformBuffer = gaunlet::Core::CreateRef<gaunlet::Graphics::UniformBuffer>(
                "EntityPropertySets",
                propertySetsUniformBufferBindingPoint,
                sizeof (PlaneQuadEntityProperties) * m_renderer.getBatchParameters().m_maxPropertySets
            );

            // Create a uniform buffer that will contain the information about the camera's frustum
            m_frustumUniformBuffer = gaunlet::Core::CreateRef<gaunlet::Graphics::UniformBuffer>(
                "CameraFrustum",
                cameraFrustumUniformBufferBindingPoint,
                sizeof (CameraFrustum)
            );

            loadShaders();
        }

        void render(gaunlet::Scene::Entity entity, const gaunlet::Core::Ref<gaunlet::Graphics::Shader>& shader) {

            auto& planeComponent = entity.getComponent<TerrainComponent>();

            // Global variables for the whole plane
            m_shaderLibrary.get("plane-faces")
                ->setUniform1f("u_triangleSize", planeComponent.m_triangleSize)
                ->setUniform1f("u_maxHeight", planeComponent.m_maxHeight)
                ->setUniform1i("u_entityId", entity.getId());

            // Global frustum data
            gaunlet::Scene::Frustum originalFrustum = planeComponent.m_camera->getFrustum();
            CameraFrustum cameraFrustum{
                {originalFrustum.m_nearPlane.m_normal, originalFrustum.m_nearPlane.m_distance},
                {originalFrustum.m_farPlane.m_normal, originalFrustum.m_farPlane.m_distance},
                {originalFrustum.m_leftPlane.m_normal, originalFrustum.m_leftPlane.m_distance},
                {originalFrustum.m_rightPlane.m_normal, originalFrustum.m_rightPlane.m_distance},
                {originalFrustum.m_bottomPlane.m_normal, originalFrustum.m_bottomPlane.m_distance},
                {originalFrustum.m_topPlane.m_normal, originalFrustum.m_topPlane.m_distance},
            };

            m_frustumUniformBuffer->setData(
                (const void*) &cameraFrustum,
                sizeof(CameraFrustum)
            );

            for (auto& quad : planeComponent.getContent()) {

                // Per-quad data
                PlaneQuadEntityProperties planeQuadEntityProperties{
                    quad.m_position,
                    quad.m_leftSizeRatio,
                    quad.m_bottomSizeRatio,
                    quad.m_rightSizeRatio,
                    quad.m_topSizeRatio
                };

                bool batched = m_renderer.submitIndexedTriangles(
                    quad.m_vertices,
                    quad.m_indices,
                    planeComponent.m_heightmap,
                    planeQuadEntityProperties
                );

                if (!batched) {
                    renderObjects(shader);
                    m_renderer.submitIndexedTriangles(
                        quad.m_vertices,
                        quad.m_indices,
                        planeComponent.m_heightmap,
                        planeQuadEntityProperties
                    );
                }

            }

            renderObjects(shader);

        }

        static unsigned int getUniformBufferCount() {return 2; }

        inline gaunlet::Graphics::ShaderLibrary& getShaders() {return m_shaderLibrary; }

    protected:

        void renderObjects(const gaunlet::Core::Ref<gaunlet::Graphics::Shader>& shader) {

            auto& entityPropertySets = m_renderer.getPropertySets();

            // Submit the entity properties to the uniform buffer
            m_propertySetsUniformBuffer->setData(
                (const void*) entityPropertySets.data(),
                sizeof(PlaneQuadEntityProperties) * entityPropertySets.size()
            );

            m_renderer.flush(shader, gaunlet::Graphics::RenderMode::Quad);

        }

        gaunlet::Graphics::BatchedRenderPass<PlaneQuadEntityProperties> m_renderer;
        gaunlet::Core::Ref<gaunlet::Graphics::UniformBuffer> m_propertySetsUniformBuffer = nullptr;
        gaunlet::Core::Ref<gaunlet::Graphics::UniformBuffer> m_frustumUniformBuffer = nullptr;

    private:

        void loadShaders() {

            std::map<gaunlet::Core::ShaderType, std::string> facesSources {
                {gaunlet::Core::ShaderType::Vertex, WORKING_DIR"/graphics/renderer/shaders/vertex.glsl"},
                {gaunlet::Core::ShaderType::TessellationControl, WORKING_DIR"/graphics/renderer/shaders/tessellation-control.glsl"},
                {gaunlet::Core::ShaderType::TessellationEvaluation, WORKING_DIR"/graphics/renderer/shaders/tessellation-evaluation.glsl"},
                {gaunlet::Core::ShaderType::Fragment, WORKING_DIR"/graphics/renderer/shaders/fragment.glsl"}
            };

            auto facesShader = m_shaderLibrary.load("plane-faces", facesSources);

            // Set a single "heighmap" texture (slot 0 is for the whiteTexture)
            facesShader->setUniform1i("heightmap", 1);

            facesShader->linkUniformBuffer(m_propertySetsUniformBuffer);
            facesShader->linkUniformBuffer(m_frustumUniformBuffer);

        }

        gaunlet::Graphics::ShaderLibrary m_shaderLibrary;

    };

}