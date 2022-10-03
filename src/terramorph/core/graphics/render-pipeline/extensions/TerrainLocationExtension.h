#pragma once

#include "gaunlet/editor/render-pipeline/RenderPipelineExtension.h"
#include "gaunlet/graphics/framebuffer/Framebuffer.h"

namespace terramorph::Core {

    class TerrainLocationExtension : public gaunlet::Editor::RenderPipelineExtension {

    public:

        TerrainLocationExtension(gaunlet::Core::Ref<gaunlet::Graphics::Framebuffer> framebuffer, unsigned int terrainLocationAttachmentIndex)
            : m_framebuffer(std::move(framebuffer)), m_terrainLocationAttachmentIndex(terrainLocationAttachmentIndex) {}

        const char * getName() override {
            return "Terrain Location";
        }

        glm::vec3 mousePickTerrainLocation(unsigned int pixelX, unsigned int pixelY) {
            return m_framebuffer->readPixel<glm::vec3>(m_terrainLocationAttachmentIndex, pixelX, pixelY);
        }

    private:

        unsigned int m_terrainLocationAttachmentIndex = 0;
        gaunlet::Core::Ref<gaunlet::Graphics::Framebuffer> m_framebuffer;

    };

}