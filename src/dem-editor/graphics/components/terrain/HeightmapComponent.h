#pragma once

#include <utility>

#include "gaunlet/graphics/texture/TextureImage2D.h"
#include "gaunlet/graphics/framebuffer/Framebuffer.h"
#include "gaunlet/core/window/Window.h"
#include "gaunlet/core/render/RenderCommand.h"
#include "gaunlet/graphics/render-pass/SimpleRenderPass.h"

namespace DemEditor {

    class HeightmapComponent {

    public:

        explicit HeightmapComponent(gaunlet::Core::Ref<gaunlet::Graphics::TextureImage2D> heightmap)
            : m_heightmap(std::move(heightmap)) {
            prepareFramebuffer();
        }

        const gaunlet::Core::Ref<gaunlet::Graphics::TextureImage2D>& getHeightmap() {
            return m_heightmap;
        }

        void update(const gaunlet::Core::Ref<gaunlet::Graphics::Shader>& shader) {

            // A full screen quad
            static std::vector<gaunlet::Graphics::Vertex> vertices = {
                {{-1, -1, 0, 1}, {0, 0, 0}, {0, 0}},
                {{1, -1, 0, 1}, {0, 0, 0}, {1, 0}},
                {{1, 1, 0, 1}, {0, 0, 0}, {1, 1}},
                {{-1, 1, 0, 1}, {0, 0, 0}, {0, 1}}
            };
            static std::vector<unsigned int> indices = {0, 1, 2, 2, 3, 0};

            m_framebuffer->bind();
            m_heightmap->activate(0);
            gaunlet::Graphics::SimpleRenderPass::renderIndexedVertices(
                vertices,
                indices,
                shader,
                gaunlet::Graphics::RenderMode::Triangle
            );

            gaunlet::Core::RenderCommand::copyColorAttachment(
                m_framebuffer->getRendererId(),
                0,
                0, 0,
                m_heightmap->getWidth(),
                m_heightmap->getHeight(),
                m_heightmap->getRendererId()
            );
            m_framebuffer->unbind();

        }

    private:

        void prepareFramebuffer() {

            // Prepare the main framebuffer, which contains the texture that we will be writing update to, and copied them from
            m_framebuffer = gaunlet::Core::CreateRef<gaunlet::Graphics::Framebuffer>(std::initializer_list<gaunlet::Graphics::FramebufferAttachmentSpec>{
                {gaunlet::Core::FramebufferAttachmentType::Color, gaunlet::Graphics::FramebufferDataFormat::RGBA, glm::vec4(0.0f, 0.0f, 0.0f, 1)}
            }, m_heightmap->getWidth(), m_heightmap->getHeight());

            // Create a temporary framebuffer, to copy the content of the original heightmap texture into the framebuffer texture
            unsigned int tempFramebufferId;
            gaunlet::Core::RenderCommand::createFramebuffer(tempFramebufferId);
            gaunlet::Core::RenderCommand::bindFramebuffer(tempFramebufferId);

            // Attach the original heightmap texture to the framebuffer
            gaunlet::Core::RenderCommand::framebufferAttach(
                tempFramebufferId,
                gaunlet::Core::TextureType::Image2D,
                gaunlet::Core::FramebufferAttachmentType::Color,
                0,
                m_heightmap->getRendererId()
            );

            // Copy from the temporary framebuffer to the main framebuffer's texture
            gaunlet::Core::RenderCommand::copyColorAttachment(
                tempFramebufferId,
                0,
                0, 0,
                m_heightmap->getWidth(), m_heightmap->getHeight(),
                m_framebuffer->getColorAttachment(0)->getRendererId()
            );

            // De-attach the original heightmap texture from the temporary framebuffer and delete the temporary framebuffer
            gaunlet::Core::RenderCommand::framebufferAttach(
                tempFramebufferId,
                gaunlet::Core::TextureType::Image2D,
                gaunlet::Core::FramebufferAttachmentType::Color,
                0,
                0 // Attach texture 0 to de-attach
            );
            gaunlet::Core::RenderCommand::unbindFramebuffer();
            gaunlet::Core::RenderCommand::deleteFramebuffer(tempFramebufferId);

        }

        gaunlet::Core::Ref<gaunlet::Graphics::TextureImage2D> m_heightmap;
        gaunlet::Core::Ref<gaunlet::Graphics::Framebuffer> m_framebuffer;

    };

}