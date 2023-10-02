#pragma once

#include "gaunlet/editor/render-pipeline/RenderPipelineExtension.h"
#include "gaunlet/graphics/framebuffer/Framebuffer.h"

namespace terramorph::Core {

    class PlanetLocationExtension : public gaunlet::Editor::RenderPipelineExtension {

    public:

        PlanetLocationExtension(gaunlet::Core::Ref<gaunlet::Graphics::Framebuffer> framebuffer, unsigned int terrainLocationAttachmentIndex)
            : m_framebuffer(std::move(framebuffer)), m_planetLocationAttachmentIndex(terrainLocationAttachmentIndex) {}

        const char * getName() override {
            return "Terrain Location";
        }

        glm::vec2 mousePickPlanetLocation(unsigned int pixelX, unsigned int pixelY) {
            return m_framebuffer->readPixel<glm::vec2>(m_planetLocationAttachmentIndex, pixelX, pixelY);
        }

    private:

        unsigned int m_planetLocationAttachmentIndex = 0;
        gaunlet::Core::Ref<gaunlet::Graphics::Framebuffer> m_framebuffer;

    };

}