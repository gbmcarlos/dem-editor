#include "dem-editor/graphics/render-pipeline/TerrainRenderPipeline.h"

#include "gaunlet/core/window/Window.h"
#include "gaunlet/prefab/render-pipelines/SceneProperties.h"
#include "gaunlet/editor/Tags.h"

namespace DemEditor {

    TerrainRenderPipeline::TerrainRenderPipeline(unsigned int uniformBufferBindingPointOffset)
        : m_terrainRenderer(3 + uniformBufferBindingPointOffset, 4 + uniformBufferBindingPointOffset) {

        prepareShaders(uniformBufferBindingPointOffset);

        auto window = gaunlet::Core::Window::getCurrentInstance();

        m_framebuffer = gaunlet::Core::CreateRef<gaunlet::Graphics::Framebuffer>(std::initializer_list<gaunlet::Graphics::FramebufferAttachmentSpec>{
            {gaunlet::Core::FramebufferAttachmentType::Color, gaunlet::Graphics::FramebufferDataFormat::RGBA, glm::vec4(0.1f, 0.1f, 0.1f, 1)},
//            {gaunlet::Core::FramebufferAttachmentType::Color, gaunlet::Graphics::FramebufferDataFormat::RGB, glm::vec3(0.0f, 0.0f, 0.0f)},
            {gaunlet::Core::FramebufferAttachmentType::Color, gaunlet::Graphics::FramebufferDataFormat::Integer, -1},
            {gaunlet::Core::FramebufferAttachmentType::Color, gaunlet::Graphics::FramebufferDataFormat::Integer, -1},
            {gaunlet::Core::FramebufferAttachmentType::DepthStencil, gaunlet::Graphics::FramebufferDataFormat::DepthStencil}
        }, window->getViewportWidth() * window->getDPI(), window->getViewportHeight() * window->getDPI());

    }

    void TerrainRenderPipeline::run(const gaunlet::Core::Ref<gaunlet::Scene::Scene> &scene, const gaunlet::Core::Ref<gaunlet::Scene::Camera> &camera, const gaunlet::Core::Ref<gaunlet::Scene::DirectionalLightComponent> &directionalLight, const gaunlet::Core::Ref<gaunlet::Scene::SkyboxComponent> &skybox) {

        // Set the draw buffers in the right order
        m_framebuffer->setDrawBuffers({
            SceneFramebufferAttachmentIndex,
            SceneEntityIdFramebufferAttachmentIndex,
//            TerrainPositionFramebufferAttachmentIndex,
            UIEntityIdFramebufferAttachmentIndex
        });

        // Clear everything
        m_framebuffer->clear();

        // Start scene doesn't do any drawing, it just sets the SceneProperties uniform buffer
        startScene(scene, camera, directionalLight ? directionalLight : gaunlet::Core::CreateRef<gaunlet::Scene::DirectionalLightComponent>());

        // The framebuffer needs to be bound before we start drawing anything
        m_framebuffer->bind();

        drawScene(scene);
        drawUI(scene);
        drawSkybox(skybox);

        m_framebuffer->unbind();

    }

    void TerrainRenderPipeline::resize(unsigned int width, unsigned int height) {

        m_framebuffer->resize(
            width,
            height
        );

    }

    const gaunlet::Core::Ref<gaunlet::Graphics::Texture> &TerrainRenderPipeline::getRenderedTexture() {
        return m_framebuffer->getColorAttachment(SceneFramebufferAttachmentIndex);
    }

    int TerrainRenderPipeline::readFramebuffer(unsigned int attachmentIndex, unsigned int x, unsigned int y) {
        return m_framebuffer->readPixel(attachmentIndex, x, y);
    }

    unsigned int TerrainRenderPipeline::getUniformBufferCount() {
        return
            TerrainRenderer::getUniformBufferCount() +
            gaunlet::Prefab::ObjectRenderers::SkyboxRenderer::getUniformBufferCount() +
            1; // The SceneProperties uniform buffer that this render pipeline manages itself
    }

    void
    TerrainRenderPipeline::startScene(const gaunlet::Core::Ref<gaunlet::Scene::Scene> &scene, const gaunlet::Core::Ref<gaunlet::Scene::Camera> &camera, const gaunlet::Core::Ref<gaunlet::Scene::DirectionalLightComponent> &directionalLight) {

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
        submitScenePlanes(scene);

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

    void TerrainRenderPipeline::submitScenePlanes(const gaunlet::Core::Ref<gaunlet::Scene::Scene> &scene) {

        // Model faces: those models that don't have the Wireframe tag
        auto facesView = scene->getRegistry().view<TerrainComponent, gaunlet::Editor::SceneEntityTag>(entt::exclude<gaunlet::Editor::WireframeModelTag>);
        for (auto e : facesView) {
            m_terrainRenderer.render(
                {e, scene},
                m_terrainRenderer.getShaders().get("plane-faces")
            );
        }

        // Model wireframes: those models that have the Wireframe tag
        gaunlet::Core::RenderCommand::setPolygonMode(gaunlet::Core::PolygonMode::Line);
        auto wireframesView = scene->getRegistry().view<TerrainComponent, gaunlet::Editor::SceneEntityTag, gaunlet::Editor::WireframeModelTag>();
        for (auto e : wireframesView) {
            m_terrainRenderer.render(
                {e, scene},
                m_terrainRenderer.getShaders().get("plane-faces")
            );
        }
        gaunlet::Core::RenderCommand::setPolygonMode(gaunlet::Core::PolygonMode::Fill);

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

}