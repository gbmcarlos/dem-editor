#pragma once

#include "gaunlet/prefab/editor-tools/EntitySelectionTool.h"
#include "dem-editor/graphics/components/TerrainComponents.h"
#include "gaunlet/core/event/events/MouseEvent.h"

namespace DemEditor {

    class TerrainSelectionTool : public gaunlet::Prefab::EditorTools::EntitySelectionTool {

    public:

        TerrainSelectionTool()
            : m_stamp(gaunlet::Core::CreateRef<gaunlet::Graphics::TextureImage2D>(ASSETS_PATH"/texture-1.jpeg")) {}

        const char* getName() override {
            return "Terrain Location";
        }

        bool onEvent(gaunlet::Core::Event &event) override {

            gaunlet::Core::EventDispatcher dispatcher(event);
            dispatcher.dispatch<gaunlet::Core::MouseButtonPress>(GL_BIND_CALLBACK_FN(TerrainSelectionTool::onMousePressEvent));
            dispatcher.dispatch<gaunlet::Core::CursorMoveEvent>(GL_BIND_CALLBACK_FN(TerrainSelectionTool::onCursorMoveEvent));
            dispatcher.dispatch<gaunlet::Core::MouseButtonRelease>(GL_BIND_CALLBACK_FN(TerrainSelectionTool::onMouseReleaseEvent));

            return true;

        }

        void onGuiRender() override {

            if (m_terrain) {
                ImGui::Text("Terrain Location: (%f %f %f)", m_terrainLocation.x, m_terrainLocation.y, m_terrainLocation.z);
            }

        }

    protected:

        gaunlet::Editor::RenderPanel* m_renderPanel = nullptr;
        gaunlet::Scene::Entity m_terrain;
        glm::vec4 m_terrainLocation;
        gaunlet::Core::Ref<gaunlet::Graphics::TextureImage2D> m_stamp;
        float m_stampUVSize = 0.05;

        bool onMousePressEvent(gaunlet::Core::MouseButtonPress& event) {

            m_renderPanel = getWorkspace()->getHoveredRenderPanel();
            if (!m_renderPanel) {
                return true;
            }

            m_terrain = mousePickTaggedEntity<TerrainComponent>(m_renderPanel, 1);

            if (!m_terrain) {
                return true;
            }

            m_terrainLocation = mousePickTerrainLocation(m_renderPanel, m_terrain.getComponent<TerrainComponent>());

            if (!m_terrain.hasComponent<StampComponent>()) {
                m_terrain.addComponent<StampComponent>(
                    glm::vec2(m_terrainLocation.x, m_terrainLocation.z),
                    m_stampUVSize,
                    m_stamp
                );
            }

            return true;

        }

        bool onCursorMoveEvent(gaunlet::Core::CursorMoveEvent& event) {

            if (m_terrain && m_terrain.hasComponent<StampComponent>()) {

                m_terrainLocation = mousePickTerrainLocation(m_renderPanel, m_terrain.getComponent<TerrainComponent>());
                auto& stamp = m_terrain.getComponent<StampComponent>();
                stamp.m_uvOrigin = {m_terrainLocation.x, m_terrainLocation.z};

            }

            return true;

        }

        bool onMouseReleaseEvent(gaunlet::Core::MouseButtonRelease& event) {

            if (m_terrain) {

                if (m_terrain.hasComponent<StampComponent>()) {
                    m_terrain.removeComponent<StampComponent>();
                }

                m_terrain = {};

            }

            m_renderPanel = nullptr;

            return true;
        }

        glm::vec4 mousePickTerrainLocation(gaunlet::Editor::RenderPanel* renderPanel, TerrainComponent& terrain) {

            unsigned int pixelPositionX = renderPanel->getMousePositionX() * gaunlet::Core::Window::getCurrentInstance()->getDPI();
            unsigned int pixelPositionY = renderPanel->getMousePositionYInverted() * gaunlet::Core::Window::getCurrentInstance()->getDPI();

            return getWorkspace()->getRenderPipeline(renderPanel->getRenderPipelineId())->getFramebuffer()->readPixel<glm::vec4>(
                3,
                pixelPositionX,
                pixelPositionY
            );

        }

    };

}