#include "terramorph/core/graphics/components/TerrainComponent.h"
#include "gaunlet/core/window/CoordinatesUtils.h"
#include "gaunlet/graphics/render-pass/SimpleRenderPass.h"
#include "gaunlet/core/render/RenderCommand.h"

namespace terramorph::Core {

    TerrainComponent::TerrainComponent(float meshWidth, float meshDepth, float maxHeight, float heightmapResolution, gaunlet::Core::Ref<gaunlet::Scene::PerspectiveCamera> camera)
        : m_meshWidth(meshWidth), m_meshDepth(meshDepth), m_maxHeight(maxHeight), m_heightmapResolution(heightmapResolution), m_camera(std::move(camera)) {
        createHeightmap();
    }

    float TerrainComponent::getHeightmapResolution() {
        return m_heightmapResolution;
    }

    float TerrainComponent::getMeshWidth() {
        return m_meshWidth;
    }

    float TerrainComponent::getMeshDepth() {
        return m_meshDepth;
    }

    const gaunlet::Core::Ref<gaunlet::Scene::PerspectiveCamera>& TerrainComponent::getCamera() {
        return m_camera;
    }

    const gaunlet::Core::Ref<gaunlet::Graphics::Texture>& TerrainComponent::getHeightmap() {
        return m_heightmap;
    }

    glm::vec3 TerrainComponent::uvLocation2WorldCoordinates(glm::vec3 location) const {
        return {
            uvWidth2WorldCoordinates(location.x),
            uvHeight2WorldCoordinates(location.y),
            uvDepth2WorldCoordinates(location.z)
        };
    }

    glm::vec2 TerrainComponent::uvLocation2WorldCoordinates(glm::vec2 location) const {
        return {
            uvWidth2WorldCoordinates(location.x),
            uvDepth2WorldCoordinates(location.y)
        };
    }

    float TerrainComponent::uvWidth2WorldCoordinates(float width) const {
        return (width * m_meshWidth) - (m_meshWidth / 2);
    }

    float TerrainComponent::uvHeight2WorldCoordinates(float height) const {
        return height * m_maxHeight;
    }

    float TerrainComponent::uvDepth2WorldCoordinates(float depth) const {
        return (depth * m_meshDepth) - (m_meshDepth / 2);
    }

    glm::vec3 TerrainComponent::worldLocation2UVCoordinates(glm::vec3 location) const {
        return {
            worldWidth2UVCoordinates(location.x),
            worldHeight2UVCoordinates(location.y),
            worldDepth2UVCoordinates(location.z)
        };
    }

    glm::vec2 TerrainComponent::worldLocation2UVCoordinates(glm::vec2 location) const {
        return {
            worldWidth2UVCoordinates(location.x),
            worldDepth2UVCoordinates(location.y)
        };
    }

    float TerrainComponent::worldWidth2UVCoordinates(float width) const {
        return (width + (m_meshWidth/2)) / m_meshWidth;
    }

    float TerrainComponent::worldHeight2UVCoordinates(float height) const {
        return height / m_maxHeight;
    }

    float TerrainComponent::worldDepth2UVCoordinates(float depth) const {
        return (depth + (m_meshDepth/2)) / m_meshDepth;
    }

    void TerrainComponent::updateHeightmap(const gaunlet::Core::Ref<gaunlet::Graphics::Shader>& shader) {
        updateHeightmap(shader, {0.5f, 0.5f}, 1.0f, 1.0f);
    }

    void TerrainComponent::updateHeightmap(const gaunlet::Core::Ref<gaunlet::Graphics::Shader>& shader, glm::vec2 uvOrigin, float uvWidth, float uvHeight) {

        auto ncQuadCorners = gaunlet::Core::CoordinatesUtils::uvQuad2nc(uvOrigin, uvWidth, uvHeight);

        std::vector<gaunlet::Graphics::Vertex> quadVertices = {
            {{ncQuadCorners[0], 0, 1}, {0, 0, 0}, {0, 0}},
            {{ncQuadCorners[1], 0, 1}, {0, 0, 0}, {1, 0}},
            {{ncQuadCorners[2], 0, 1}, {0, 0, 0}, {1, 1}},
            {{ncQuadCorners[3], 0, 1}, {0, 0, 0}, {0, 1}}
        };
        static std::vector<unsigned int> quadIndices = {0, 1, 2, 2, 3, 0};

        m_heightmapFramebuffer->bind();
        m_heightmap->activate(0);
        gaunlet::Graphics::SimpleRenderPass::renderIndexedVertices(
            quadVertices,
            quadIndices,
            shader,
            gaunlet::Graphics::RenderMode::Triangle
        );

        swapHeightmap(uvOrigin, uvWidth, uvHeight);

        m_heightmapFramebuffer->unbind();

    }

    void TerrainComponent::resetHeightmap() {

        m_heightmapFramebuffer->bind();

        m_heightmapFramebuffer->clear();
        swapHeightmap({0.5f, 0.5f}, 1.0f, 1.0f);

        m_heightmapFramebuffer->unbind();

    }

    void TerrainComponent::swapHeightmap(glm::vec2 uvOrigin, float uvWidth, float uvHeight) {

        float heightmapWidth = m_meshWidth * m_heightmapResolution;
        float heightmapHeight = m_meshDepth * m_heightmapResolution;

        auto pixelLeft = (unsigned int) std::max(0.0f, std::floor(heightmapWidth * (uvOrigin.x - uvWidth)));
        auto pixelBottom = (unsigned int) std::max(0.0f, std::floor(heightmapHeight * (uvOrigin.y - uvHeight)));

        auto pixelRight = (unsigned int) std::min(heightmapWidth, std::ceil(heightmapWidth * (uvOrigin.x + uvWidth)));
        auto pixelTop = (unsigned int) std::min(heightmapHeight, std::ceil(heightmapHeight * (uvOrigin.y + uvHeight)));

        auto stampPixelWidth = pixelRight - pixelLeft;
        auto stampPixelHeight = pixelTop - pixelBottom;

        gaunlet::Core::RenderCommand::copyColorAttachment(
            m_heightmapFramebuffer->getRendererId(),
            0,
            pixelLeft, pixelBottom, // left-bottom corner of the attachment texture to copy from
            pixelLeft, pixelBottom, // left-bottom corner of the texture to copy to
            stampPixelWidth, stampPixelHeight, // width and height of the rectangle of pixels to copy
            m_heightmap->getRendererId()
        );

    }

    void TerrainComponent::createHeightmap() {

        m_heightmapFramebuffer = gaunlet::Core::CreateRef<gaunlet::Graphics::Framebuffer>(
            m_meshWidth * m_heightmapResolution, m_meshDepth * m_heightmapResolution
        );

        // The deformation heightmap
        m_heightmapFramebuffer->addColorAttachment<float>(
            gaunlet::Graphics::ColorAttachmentSpec::Channels::CHANNELS_1,
            gaunlet::Graphics::ColorAttachmentSpec::Type::TYPE_UNI,
            gaunlet::Graphics::ColorAttachmentSpec::Size::SIZE_16,
            0.0f
        );

        // The main heightmap
        m_heightmapFramebuffer->addColorAttachment<float>(
            gaunlet::Graphics::ColorAttachmentSpec::Channels::CHANNELS_1,
            gaunlet::Graphics::ColorAttachmentSpec::Type::TYPE_UNI,
            gaunlet::Graphics::ColorAttachmentSpec::Size::SIZE_16,
            0.0f
        );

        m_heightmapFramebuffer->clear();

        // Save the second attachment as the heightmap and deattach it from the framebuffer
        m_heightmap = m_heightmapFramebuffer->getColorAttachment(1);
        m_heightmapFramebuffer->deAttachColor(1);

    }

}