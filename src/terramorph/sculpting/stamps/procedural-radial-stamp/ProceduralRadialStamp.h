#pragma once

#include "terramorph/core/tools/terrain-deformation/stamp-deformation/DeformationBrush.h"

#include "terramorph/pch.h"

namespace terramorph::Sculpting {

    class ProceduralRadialStamp : public Core::DeformationBrush {

    public:

        ProceduralRadialStamp() {

            m_framebuffer = gaunlet::Core::CreateRef<gaunlet::Graphics::Framebuffer>(
                200.0f, 200.0f
            );

            m_framebuffer->addColorAttachment<glm::vec3>(
                gaunlet::Graphics::ColorAttachmentSpec::Channels::CHANNELS_3,
                gaunlet::Graphics::ColorAttachmentSpec::Type::TYPE_SNI,
                gaunlet::Graphics::ColorAttachmentSpec::Size::SIZE_16,
                glm::vec3(0.0f, 0.0f, 0.0f)
            );

            m_framebuffer->recreate();

            prepareShaders();
            renderStamp();

        }

        const gaunlet::Core::Ref<gaunlet::Graphics::Texture>& getBrushStampTexture() override {
            return m_framebuffer->getColorAttachment(0);
        }

        void onUpdate(gaunlet::Core::TimeStep timeStep) override {

            if (m_editing) {
                renderStamp();
            }

        }

        void onGuiRender() override {

            if (m_editing) {
                if (ImGui::Button("Exit")) {
                    m_editing = false;
                }
            } else {
                if (ImGui::Button("Edit Brush")) {
                    m_editing = true;
                }
            }

            ImGui::BeginDisabled(!m_editing);

            ImGui::SliderFloat("Min Value", &m_minValue, 0.0f, 1.0f);
            ImGui::SliderFloat("Max Value", &m_maxValue, 0.0f, 1.0f);
            ImGui::SliderFloat("Step Min", &m_stepMin, 0.0f, 1.0f);
            ImGui::SliderFloat("Step Max", &m_stepMax, 0.0f, 1.0f);

            ImGui::EndDisabled();

            ImGui::Image(
                (void *)(intptr_t)m_framebuffer->getColorAttachment(0)->getRendererId(),
                ImVec2(200, 200),
                ImVec2(0, 1), ImVec2(1, 0)
            );

        }

    private:

        void renderStamp() {

            // A full screen quad
            static std::vector<gaunlet::Graphics::Vertex> vertices = {
                {{-1, -1, 0, 1}, {0, 0, 0}, {0, 0}},
                {{1, -1, 0, 1}, {0, 0, 0}, {1, 0}},
                {{1, 1, 0, 1}, {0, 0, 0}, {1, 1}},
                {{-1, 1, 0, 1}, {0, 0, 0}, {0, 1}}
            };
            static std::vector<unsigned int> indices = {0, 1, 2, 2, 3, 0};

            m_framebuffer->bind();
            m_framebuffer->clear();
            auto& shader = m_shaderLibrary.get("stamp-editor");
            shader->setUniform1f("minValue", m_minValue);
            shader->setUniform1f("maxValue", m_maxValue);
            shader->setUniform1f("stepMin", m_stepMin);
            shader->setUniform1f("stepMax", m_stepMax);

            gaunlet::Graphics::SimpleRenderPass::renderIndexedVertices(
                vertices,
                indices,
                shader,
                gaunlet::Graphics::RenderMode::Triangle
            );
            m_framebuffer->unbind();

        }

        void prepareShaders() {

            std::map<gaunlet::Core::ShaderType, std::string> editorSources {
                {gaunlet::Core::ShaderType::Vertex, GL_PREFABS_PATH"/shaders/pass-through-vertex.glsl"},
                {gaunlet::Core::ShaderType::Fragment, WORKING_DIR"/sculpting/stamps/procedural-radial-stamp/shaders/fragment.glsl"}
            };

            auto shader = m_shaderLibrary.load("stamp-editor", editorSources);

        }

    private:

        float m_minValue = 0.5f;
        float m_maxValue = 1.0f;
        float m_stepMin = 0.3f;
        float m_stepMax = 1.0f;
        bool m_editing = false;
        gaunlet::Core::Ref<gaunlet::Graphics::Framebuffer> m_framebuffer = nullptr;
        gaunlet::Graphics::ShaderLibrary m_shaderLibrary;

    };

}