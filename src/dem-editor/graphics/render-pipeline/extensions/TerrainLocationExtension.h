#pragma once

#include <utility>

#include "gaunlet/editor/render-pipeline/RenderPipelineExtension.h"
#include "gaunlet/graphics/framebuffer/Framebuffer.h"

namespace DemEditor {

    class TerrainLocationExtension : public gaunlet::Editor::RenderPipelineExtension {

    public:

        TerrainLocationExtension(gaunlet::Core::Ref<gaunlet::Graphics::Framebuffer> framebuffer, unsigned int terrainLocationAttachmentIndex)
            : m_framebuffer(std::move(framebuffer)), m_terrainLocationAttachmentIndex(terrainLocationAttachmentIndex) {}

        const char * getName() override {
            return "Terrain Location";
        }

        glm::vec4 mousePickTerrainLocation(unsigned int pixelX, unsigned int pixelY) {
            return m_framebuffer->readPixel<glm::vec4>(m_terrainLocationAttachmentIndex, pixelX, pixelY);
        }

    private:

        unsigned int m_terrainLocationAttachmentIndex = 0;
        gaunlet::Core::Ref<gaunlet::Graphics::Framebuffer> m_framebuffer;

    };

}