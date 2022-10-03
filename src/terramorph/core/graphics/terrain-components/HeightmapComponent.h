#pragma once

#include <utility>

#include "gaunlet/graphics/texture/TextureImage2D.h"
#include "gaunlet/graphics/framebuffer/Framebuffer.h"
#include "gaunlet/core/window/Window.h"
#include "gaunlet/core/render/RenderCommand.h"
#include "gaunlet/graphics/render-pass/SimpleRenderPass.h"

namespace terramorph::Core {

    class HeightmapComponent {

    public:

        explicit HeightmapComponent(unsigned int size)
            : m_size(size) {
            recreate();
        }

        const gaunlet::Core::Ref<gaunlet::Graphics::Texture>& getHeightmap() {
            return m_heightmap;
        }

        unsigned int getSize() {
            return m_size;
        }

        void updateQuad(const gaunlet::Core::Ref<gaunlet::Graphics::Shader>& shader, glm::vec2 uvOrigin, float uvSize) {
            update(shader, uvOrigin, uvSize, uvSize);
        }

        void updateFull(const gaunlet::Core::Ref<gaunlet::Graphics::Shader>& shader) {
            update(shader, {0.5f, 0.5f}, 1.0f, 1.0f);
        }

    protected:

        void update(const gaunlet::Core::Ref<gaunlet::Graphics::Shader>& shader, glm::vec2 uvOrigin, float width, float height) {

            auto ncQuadCorners = gaunlet::Core::CoordinatesUtils::uvQuad2nc(uvOrigin, width, height);

            std::vector<gaunlet::Graphics::Vertex> quadVertices = {
                {{ncQuadCorners[0], 0, 1}, {0, 0, 0}, {0, 0}},
                {{ncQuadCorners[1], 0, 1}, {0, 0, 0}, {1, 0}},
                {{ncQuadCorners[2], 0, 1}, {0, 0, 0}, {1, 1}},
                {{ncQuadCorners[3], 0, 1}, {0, 0, 0}, {0, 1}}
            };
            static std::vector<unsigned int> quadIndices = {0, 1, 2, 2, 3, 0};

            m_framebuffer->bind();
            m_heightmap->activate(0);
            gaunlet::Graphics::SimpleRenderPass::renderIndexedVertices(
                quadVertices,
                quadIndices,
                shader,
                gaunlet::Graphics::RenderMode::Triangle
            );

            unsigned int pixelLeft = m_size * (unsigned int) (uvOrigin.x - width);
            unsigned int pixelBottom = m_size * (unsigned int) (uvOrigin.y - height);

            gaunlet::Core::RenderCommand::copyColorAttachment(
                m_framebuffer->getRendererId(),
                0,
                pixelLeft, pixelBottom,
                pixelLeft, pixelBottom,
                m_heightmap->getWidth(),
                m_heightmap->getHeight(),
                m_heightmap->getRendererId()
            );
            m_framebuffer->unbind();

        }

    private:

        void recreate() {

            // Prepare the main framebuffer, which contains the heightmap, and the texture that we will be writing updates to, and copying them from
            m_framebuffer = gaunlet::Core::CreateRef<gaunlet::Graphics::Framebuffer>(std::initializer_list<gaunlet::Graphics::FramebufferAttachmentSpec>{
                {gaunlet::Core::FramebufferAttachmentType::Color, gaunlet::Graphics::FramebufferDataFormat::R_Float, 0.0f},
                {gaunlet::Core::FramebufferAttachmentType::Color, gaunlet::Graphics::FramebufferDataFormat::R_Float, 0.0f}
            }, m_size, m_size);

            m_framebuffer->clear();

            // Save the second attachment as the heightmap and deattach it from the framebuffer
            m_heightmap = m_framebuffer->getColorAttachment(1);

        }

        gaunlet::Core::Ref<gaunlet::Graphics::Texture> m_heightmap;
        unsigned int m_size;
        gaunlet::Core::Ref<gaunlet::Graphics::Framebuffer> m_framebuffer;

    };

}