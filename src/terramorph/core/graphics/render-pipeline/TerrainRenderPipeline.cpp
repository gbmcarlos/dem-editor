#include "terramorph/core/graphics/render-pipeline/TerrainRenderPipeline.h"

#include "gaunlet/core/window/Window.h"
#include "gaunlet/prefab/render-pipelines/SceneProperties.h"
#include "gaunlet/editor/Tags.h"
#include "gaunlet/prefab/render-pipeline-extensions/EntitySelectionExtension.h"
#include "terramorph/core/graphics/render-pipeline/extensions/PlanetLocationExtension.h"
#include "terramorph/core/graphics/components/PlanetComponent.h"
#include "terramorph/core/graphics/components/TerrainComponent.h"

namespace terramorph::Core {

    TerrainRenderPipeline::TerrainRenderPipeline(const char* terrainEntityName, gaunlet::Core::Ref<gaunlet::Scene::DirectionalLightComponent> directionalLight, gaunlet::Core::Ref<gaunlet::Scene::SkyboxComponent> skybox, unsigned int uniformBufferBindingPointOffset)
        : m_terrainEntityName(terrainEntityName), m_directionalLight(std::move(directionalLight)), m_skybox(std::move(skybox)), m_terrainRenderer(3 + uniformBufferBindingPointOffset, 4 + uniformBufferBindingPointOffset) {

        prepareShaders(uniformBufferBindingPointOffset);
        prepareFramebuffer();

        addExtension<gaunlet::Prefab::RenderPipelineExtensions::EntitySelectionExtension>(gaunlet::Core::CreateRef<gaunlet::Prefab::RenderPipelineExtensions::EntitySelectionExtension>(
            m_framebuffer,
            1, 2
        ));

        addExtension<PlanetLocationExtension>(gaunlet::Core::CreateRef<PlanetLocationExtension>(
            m_framebuffer,
            3
        ));

    }

    void TerrainRenderPipeline::run(const gaunlet::Core::Ref<gaunlet::Scene::Scene> &scene, const gaunlet::Core::Ref<gaunlet::Scene::Camera> &camera) {

        // Set the draw buffers in the right order
        m_framebuffer->setDrawBuffers({
            SceneFramebufferAttachmentIndex,
            SceneEntityIdFramebufferAttachmentIndex,
            TerrainPositionFramebufferAttachmentIndex,
            UIEntityIdFramebufferAttachmentIndex
        });

        // Clear everything
        m_framebuffer->clear();

        // Start scene doesn't do any drawing, it just sets the SceneProperties uniform buffer
        startScene(scene, camera, m_directionalLight ? m_directionalLight : gaunlet::Core::CreateRef<gaunlet::Scene::DirectionalLightComponent>());

        // The framebuffer needs to be bound before we start drawing anything
        m_framebuffer->bind();

        drawScene(scene);
        drawUI(scene);
        drawSkybox(m_skybox);

        m_framebuffer->unbind();

    }

    void TerrainRenderPipeline::resize(unsigned int width, unsigned int height) {

        m_framebuffer->resize(
            width,
            height
        );

    }

    const gaunlet::Core::Ref<gaunlet::Graphics::TextureImage2D> &TerrainRenderPipeline::getRenderTarget() {
        return m_framebuffer->getColorAttachment(SceneFramebufferAttachmentIndex);
    }

    unsigned int TerrainRenderPipeline::getUniformBufferCount() {
        return
            TerrainRenderer::getUniformBufferCount() +
            gaunlet::Prefab::ObjectRenderers::SkyboxRenderer::getUniformBufferCount() +
            1; // The SceneProperties uniform buffer that this render pipeline manages itself
    }

    void TerrainRenderPipeline::startScene(const gaunlet::Core::Ref<gaunlet::Scene::Scene> &scene, const gaunlet::Core::Ref<gaunlet::Scene::Camera> &camera, const gaunlet::Core::Ref<gaunlet::Scene::DirectionalLightComponent> &directionalLight) {

        gaunlet::Prefab::RenderPipelines::SceneProperties sceneProperties(
            camera->getViewMatrix(), camera->getProjectionMatrix(),
            directionalLight->m_color, directionalLight->m_direction,
            directionalLight->m_ambientIntensity, directionalLight->m_diffuseIntensity
        );

        unsigned int viewportX0, viewportY0, viewportX1, viewportY1;
        gaunlet::Core::RenderCommand::getViewport(
            viewportX0, viewportY0, viewportX1, viewportY1
        );

        sceneProperties.m_viewport = {viewportX0, viewportY0, viewportX1, viewportY1};

        m_scenePropertiesUniformBuffer->setData(
            (const void*) &sceneProperties,
            sizeof(gaunlet::Prefab::RenderPipelines::SceneProperties)
        );

    }

    void TerrainRenderPipeline::drawScene(const gaunlet::Core::Ref<gaunlet::Scene::Scene> &scene) {

        // Normal depth testing
        gaunlet::Core::RenderCommand::setDepthFunction(gaunlet::Core::DepthStencilFunction::Less);

        // Always write 1 to the stencil buffer
        gaunlet::Core::RenderCommand::setStencilFunction(
            gaunlet::Core::DepthStencilFunction::Always, 1
        );

        // Write to the stencil buffer only when we draw
        gaunlet::Core::RenderCommand::setStencilOperation(
            true,
            gaunlet::Core::StencilOperation::Keep,
            gaunlet::Core::StencilOperation::Keep,
            gaunlet::Core::StencilOperation::Replace
        );

        // Then draw the objects
        submitSceneTerrains(scene);

    }

    void TerrainRenderPipeline::drawUI(const gaunlet::Core::Ref<gaunlet::Scene::Scene> &scene) {}

    void TerrainRenderPipeline::drawSkybox(const gaunlet::Core::Ref<gaunlet::Scene::SkyboxComponent> &skybox) {

        if (skybox == nullptr || !skybox->m_cubeMap) {
            return;
        }

        // We don't care about depth
        gaunlet::Core::RenderCommand::setDepthFunction(gaunlet::Core::DepthStencilFunction::Always);

        // Draw only wherever we didn't draw anything before
        gaunlet::Core::RenderCommand::setStencilFunction(
            gaunlet::Core::DepthStencilFunction::NotEqual, 1
        );

        // Don't write to the stencil buffer
        gaunlet::Core::RenderCommand::setStencilOperation(
            false,
            gaunlet::Core::StencilOperation::Keep,
            gaunlet::Core::StencilOperation::Keep,
            gaunlet::Core::StencilOperation::Keep
        );

        renderSkybox(skybox);

    }

    void TerrainRenderPipeline::submitSceneTerrains(const gaunlet::Core::Ref<gaunlet::Scene::Scene> &scene) {

        auto terrainEntity = scene->getEntity(m_terrainEntityName);

        if (terrainEntity.hasComponent<PlanetComponent>() && terrainEntity.hasComponent<TerrainComponent>()) {

            auto& shader = m_terrainRenderer.getShaders().get("plane-faces");

            if (terrainEntity.hasComponent<gaunlet::Editor::WireframeModelTag>()) {
                gaunlet::Core::RenderCommand::setPolygonMode(gaunlet::Core::PolygonMode::Line);
                m_terrainRenderer.render(terrainEntity, shader);
                gaunlet::Core::RenderCommand::setPolygonMode(gaunlet::Core::PolygonMode::Fill);
            } else {
                m_terrainRenderer.render(terrainEntity, shader);
            }

        }

    }

    void TerrainRenderPipeline::renderSkybox(const gaunlet::Core::Ref<gaunlet::Scene::SkyboxComponent> &skybox) {

        if (skybox->m_cubeMap) {
            m_skyboxRenderer.render(
                skybox->m_cubeMap,
                m_skyboxRenderer.getShaders().get("skybox")
            );
        }

    }

    void TerrainRenderPipeline::prepareShaders(unsigned int uniformBufferBindingPointOffset) {

        // Create the Uniform Buffer for the Scene Properties, which will be linked to every shader
        m_scenePropertiesUniformBuffer = gaunlet::Core::CreateRef<gaunlet::Graphics::UniformBuffer>(
            "SceneProperties",
            0 + uniformBufferBindingPointOffset,
            sizeof (gaunlet::Prefab::RenderPipelines::SceneProperties)
        );

        m_skyboxRenderer.getShaders().get("skybox")->linkUniformBuffer(m_scenePropertiesUniformBuffer);
        m_terrainRenderer.getShaders().get("plane-faces")->linkUniformBuffer(m_scenePropertiesUniformBuffer);

    }

    void TerrainRenderPipeline::prepareFramebuffer() {

        auto window = gaunlet::Core::Window::getCurrentInstance();

        m_framebuffer = gaunlet::Core::CreateRef<gaunlet::Graphics::Framebuffer>(
            window->getViewportWidth() * window->getDPI(),
            window->getViewportHeight() * window->getDPI()
        );

        // The render target
        m_framebuffer->addColorAttachment<glm::vec4>(
            gaunlet::Graphics::ColorAttachmentSpec::Channels::CHANNELS_4,
            gaunlet::Graphics::ColorAttachmentSpec::Type::TYPE_UNI,
            gaunlet::Graphics::ColorAttachmentSpec::Size::SIZE_8,
            glm::vec4(0.0f, 0.0f, 0.0f, 1)
        );

        // For scene entity ids
        m_framebuffer->addColorAttachment<int>(
            gaunlet::Graphics::BaseColorAttachmentSpec::Channels::CHANNELS_1,
            gaunlet::Graphics::BaseColorAttachmentSpec::Type::TYPE_SI,
            gaunlet::Graphics::BaseColorAttachmentSpec::Size::SIZE_32,
            -1
        );

        // For UI entity ids
        m_framebuffer->addColorAttachment<int>(
            gaunlet::Graphics::BaseColorAttachmentSpec::Channels::CHANNELS_1,
            gaunlet::Graphics::BaseColorAttachmentSpec::Type::TYPE_SI,
            gaunlet::Graphics::BaseColorAttachmentSpec::Size::SIZE_32,
            -1
        );

        // For the terrain location
        m_framebuffer->addColorAttachment<glm::vec3>(
            gaunlet::Graphics::BaseColorAttachmentSpec::Channels::CHANNELS_2,
            gaunlet::Graphics::BaseColorAttachmentSpec::Type::TYPE_UNI,
            gaunlet::Graphics::BaseColorAttachmentSpec::Size::SIZE_16,
            glm::vec3(0.0f, 0.0f, 0.0f)
        );

        m_framebuffer->setDepthStencilAttachment(1.0f, 0);

    }

}