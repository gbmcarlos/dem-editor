#pragma once

#include "gaunlet/scene/renderer/ObjectRenderer.h"
#include "gaunlet/prefab/object-renderers/model-renderer/ModelRenderer.h"
#include "gaunlet/graphics/render-pass/SimpleRenderPass.h"
#include "terramorph/graphics/components/terrain/PlaneComponent.h"
#include "terramorph/graphics/components/terrain/StampComponent.h"
#include "terramorph/graphics/components/terrain/HeightmapComponent.h"
#include "gaunlet/scene/camera/Camera.h"

#include "gaunlet/pch.h"

namespace terramorph {

    struct QuadProperties {

        QuadProperties() = default;

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
        CameraFrustum m_cameraFrustum;
        float m_triangleSize;
        float m_maxHeight;
        glm::vec2 m_stampOrigin = {0, 0};
        float m_brushStampSize = 0;
        int m_entityId;
        float m_terrainSize;
    };

    class TerrainRenderer {

    public:

        TerrainRenderer(unsigned int quadPropertySetsUniformBufferBindingPoint, unsigned int terrainPropertiesUniformBufferBindingPoint)
            : m_renderer({100000, 600000, 10, 1000}) {

            // Create a uniform buffer that will contain the properties of every object, and will be linked to the shader
            m_quadPropertySetsUniformBuffer = gaunlet::Core::CreateRef<gaunlet::Graphics::UniformBuffer>(
                "EntityPropertySets",
                quadPropertySetsUniformBufferBindingPoint,
                sizeof (QuadProperties) * m_renderer.getBatchParameters().m_maxPropertySets
            );

            // Create a uniform buffer that will contain the information about the camera's frustum
            m_terrainPropertiesUniformBuffer = gaunlet::Core::CreateRef<gaunlet::Graphics::UniformBuffer>(
                "TerrainProperties",
                terrainPropertiesUniformBufferBindingPoint,
                sizeof (TerrainProperties)
            );

            loadShaders();
        }

        void render(gaunlet::Scene::Entity entity, const gaunlet::Core::Ref<gaunlet::Graphics::Shader>& shader) {

            auto terrainProperties = getTerrainProperties(entity);

            auto& planeComponent = entity.getComponent<PlaneComponent>();

            m_terrainPropertiesUniformBuffer->setData(
                (const void*) &terrainProperties,
                sizeof(TerrainProperties)
            );

            for (auto& quad : planeComponent.getContent()) {

                auto planeQuadEntityProperties = getQuadProperties(quad);

                bool batched = m_renderer.submitIndexedTriangles(
                    quad.m_vertices,
                    quad.m_indices,
                    nullptr,
                    planeQuadEntityProperties
                );

                if (!batched) {
                    renderObjects(shader);
                    m_renderer.submitIndexedTriangles(
                        quad.m_vertices,
                        quad.m_indices,
                        nullptr,
                        planeQuadEntityProperties
                    );
                }

            }

            renderObjects(shader);

        }

        static unsigned int getUniformBufferCount() {return 2; }

        inline gaunlet::Graphics::ShaderLibrary& getShaders() {return m_shaderLibrary; }

    protected:

        TerrainProperties getTerrainProperties(gaunlet::Scene::Entity entity) {

            auto& planeComponent = entity.getComponent<PlaneComponent>();

            if (entity.hasComponent<HeightmapComponent>()) {
                entity.getComponent<HeightmapComponent>().getHeightmap()->activate(1);
            }

            gaunlet::Scene::Frustum originalFrustum = planeComponent.m_camera->getFrustum();
            CameraFrustum cameraFrustum{
                {originalFrustum.m_nearPlane.m_normal, originalFrustum.m_nearPlane.m_distance},
                {originalFrustum.m_farPlane.m_normal, originalFrustum.m_farPlane.m_distance},
                {originalFrustum.m_leftPlane.m_normal, originalFrustum.m_leftPlane.m_distance},
                {originalFrustum.m_rightPlane.m_normal, originalFrustum.m_rightPlane.m_distance},
                {originalFrustum.m_bottomPlane.m_normal, originalFrustum.m_bottomPlane.m_distance},
                {originalFrustum.m_topPlane.m_normal, originalFrustum.m_topPlane.m_distance},
            };

            TerrainProperties terrainProperties{
                cameraFrustum,
                planeComponent.m_triangleSize,
                planeComponent.m_maxHeight
            };

            if (entity.hasComponent<StampComponent>()) {

                auto& stampComponent = entity.getComponent<StampComponent>();

                terrainProperties.m_stampOrigin = stampComponent.m_uvOrigin;
                terrainProperties.m_brushStampSize = stampComponent.m_uvSize;

                stampComponent.m_stampTexture->activate(2);

            }

            terrainProperties.m_entityId = entity.getId();
            terrainProperties.m_terrainSize = planeComponent.m_size;

            return terrainProperties;


        }

        QuadProperties getQuadProperties(PlaneQuad& quad) {

            return {
                quad.m_position,
                quad.m_leftSizeRatio,
                quad.m_bottomSizeRatio,
                quad.m_rightSizeRatio,
                quad.m_topSizeRatio
            };

        }

        void renderObjects(const gaunlet::Core::Ref<gaunlet::Graphics::Shader>& shader) {

            auto& entityPropertySets = m_renderer.getPropertySets();

            // Submit the entity properties to the uniform buffer
            m_quadPropertySetsUniformBuffer->setData(
                (const void*) entityPropertySets.data(),
                sizeof(QuadProperties) * entityPropertySets.size()
            );

            m_renderer.flush(shader, gaunlet::Graphics::RenderMode::Quad);

        }

        gaunlet::Graphics::BatchedRenderPass<QuadProperties> m_renderer;
        gaunlet::Core::Ref<gaunlet::Graphics::UniformBuffer> m_quadPropertySetsUniformBuffer = nullptr;
        gaunlet::Core::Ref<gaunlet::Graphics::UniformBuffer> m_terrainPropertiesUniformBuffer = nullptr;

    private:

        void loadShaders() {

            std::map<gaunlet::Core::ShaderType, std::string> facesSources {
                {gaunlet::Core::ShaderType::Vertex, WORKING_DIR"/graphics/renderer/shaders/vertex.glsl"},
                {gaunlet::Core::ShaderType::TessellationControl, WORKING_DIR"/graphics/renderer/shaders/tessellation-control.glsl"},
                {gaunlet::Core::ShaderType::TessellationEvaluation, WORKING_DIR"/graphics/renderer/shaders/tessellation-evaluation.glsl"},
                {gaunlet::Core::ShaderType::Fragment, WORKING_DIR"/graphics/renderer/shaders/fragment.glsl"}
            };

            auto facesShader = m_shaderLibrary.load("plane-faces", facesSources);

            // Set "heightmap" and "stamp" textures (slot 0 is for the whiteTexture)
            facesShader->setUniform1i("heightmap", 1);
            facesShader->setUniform1i("brushStamp", 2);

            facesShader->linkUniformBuffer(m_quadPropertySetsUniformBuffer);
            facesShader->linkUniformBuffer(m_terrainPropertiesUniformBuffer);

        }

        gaunlet::Graphics::ShaderLibrary m_shaderLibrary;

    };

}