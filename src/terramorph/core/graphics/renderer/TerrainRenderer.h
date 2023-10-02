#pragma once

#include "gaunlet/scene/renderer/ObjectRenderer.h"
#include "gaunlet/prefab/object-renderers/model-renderer/ModelRenderer.h"
#include "gaunlet/graphics/render-pass/SimpleRenderPass.h"
#include "gaunlet/scene/camera/Camera.h"
#include "terramorph/core/graphics/renderer/Properties.h"

#include "gaunlet/pch.h"

namespace terramorph::Core {

    class TerrainRenderer {

    public:

        TerrainRenderer(unsigned int quadPropertySetsUniformBufferBindingPoint, unsigned int terrainPropertiesUniformBufferBindingPoint)
            : m_renderer({100000, 600000, 10, 1000}) {

            // Create a uniform buffer that will contain the properties of every quad
            m_quadPropertySetsUniformBuffer = gaunlet::Core::CreateRef<gaunlet::Graphics::UniformBuffer>(
                "EntityPropertySets",
                quadPropertySetsUniformBufferBindingPoint,
                sizeof (QuadProperties) * m_renderer.getBatchParameters().m_maxPropertySets
            );

            // Create a uniform buffer that will contain the information about the terrain
            m_terrainPropertiesUniformBuffer = gaunlet::Core::CreateRef<gaunlet::Graphics::UniformBuffer>(
                "TerrainProperties",
                terrainPropertiesUniformBufferBindingPoint,
                sizeof (TerrainProperties)
            );

            loadShaders();
        }

        void render(gaunlet::Scene::Entity entity, const gaunlet::Core::Ref<gaunlet::Graphics::Shader>& shader) {

            auto& planetComponent = entity.getComponent<PlanetComponent>();
            auto content = planetComponent.getContent();

            auto& terrainComponent = entity.getComponent<TerrainComponent>();
            terrainComponent.getHeightmap()->activate(1);

            TerrainProperties terrainProperties(planetComponent, terrainComponent);
            terrainProperties.m_entityId = entity.getId();

            // If there is a stamp, take the information about it, and activate its texture
            if (entity.hasComponent<StampComponent>()) {

                auto& stampComponent = entity.getComponent<StampComponent>();

                terrainProperties.m_stampUvOrigin = stampComponent.m_origin;
                terrainProperties.m_stampUvWidth = stampComponent.m_width;
                terrainProperties.m_stampUvHeight = stampComponent.m_height;
                stampComponent.m_stampTexture->activate(2);
            }

            m_terrainPropertiesUniformBuffer->setData(
                (const void*) &terrainProperties,
                sizeof(TerrainProperties)
            );

            for (auto& quad : content) {

                QuadProperties quadProperties(quad);

                bool batched = m_renderer.submitIndexedTriangles(
                    quad.m_vertices,
                    quad.m_indices,
                    nullptr,
                    quadProperties
                );

                if (!batched) {
                    renderObjects(shader);
                    m_renderer.submitIndexedTriangles(
                        quad.m_vertices,
                        quad.m_indices,
                        nullptr,
                        quadProperties
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
                {gaunlet::Core::ShaderType::Vertex, WORKING_DIR"/core/graphics/renderer/shaders/vertex.glsl"},
                {gaunlet::Core::ShaderType::TessellationControl, WORKING_DIR"/core/graphics/renderer/shaders/tessellation-control.glsl"},
                {gaunlet::Core::ShaderType::TessellationEvaluation, WORKING_DIR"/core/graphics/renderer/shaders/tessellation-evaluation.glsl"},
                {gaunlet::Core::ShaderType::Fragment, WORKING_DIR"/core/graphics/renderer/shaders/fragment.glsl"}
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