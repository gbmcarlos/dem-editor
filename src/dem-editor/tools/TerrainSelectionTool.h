#pragma once

#include "gaunlet/prefab/editor-tools/EntitySelectionTool.h"
#include "dem-editor/graphics/components/TerrainComponents.h"
#include "gaunlet/core/event/events/MouseEvent.h"
#include "dem-editor/graphics/render-pipeline/extensions/TerrainLocationExtension.h"

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

            return true;

        }

        void onUpdate(gaunlet::Core::TimeStep timeStep) {

            // Get the render panel
            m_renderPanel = getWorkspace()->getHoveredRenderPanel();
            if (!m_renderPanel || !m_renderPanel->getRenderPipeline()->hasExtension<TerrainLocationExtension>()) {
                return;
            }

            // Get the terrain
            m_plane = mousePickTaggedEntity<PlaneComponent>(m_renderPanel, gaunlet::Prefab::RenderPipelineExtensions::EntitySelectionExtension::EntityLayer::SceneLayer);
            if (!m_plane || !m_plane.hasComponent<HeightmapComponent>()) {
                return;
            }

            auto& heightmap = m_plane.getComponent<HeightmapComponent>();

            if (gaunlet::Core::Input::isMouseButtonPressed(0)) {
//                heightmap.update();
            }

        }

        void onGuiRender() override {

            if (m_plane) {
                ImGui::Text("Terrain Location: (%f %f %f)", m_terrainLocation.x, m_terrainLocation.y, m_terrainLocation.z);
            }

        }

    protected:

        gaunlet::Editor::RenderPanel* m_renderPanel = nullptr;
        gaunlet::Scene::Entity m_plane;
        glm::vec4 m_terrainLocation;
        gaunlet::Core::Ref<gaunlet::Graphics::TextureImage2D> m_stamp;
        float m_stampUVSize = 0.05;

        bool onMousePressEvent(gaunlet::Core::MouseButtonPress& event) {

            // Get the render panel
            m_renderPanel = getWorkspace()->getHoveredRenderPanel();
            if (!m_renderPanel || !m_renderPanel->getRenderPipeline()->hasExtension<TerrainLocationExtension>()) {
                return true;
            }

            // Get the terrain
            m_plane = mousePickTaggedEntity<PlaneComponent>(m_renderPanel, gaunlet::Prefab::RenderPipelineExtensions::EntitySelectionExtension::EntityLayer::SceneLayer);
            if (!m_plane || !m_plane.hasComponent<HeightmapComponent>()) {
                return true;
            }

            auto& heightmap = m_plane.getComponent<HeightmapComponent>();

            heightmap.update();

            return true;

        }

        bool onCursorMoveEvent(gaunlet::Core::CursorMoveEvent& event) {

            m_renderPanel = getWorkspace()->getHoveredRenderPanel();
            if (!m_renderPanel || !m_renderPanel->getRenderPipeline()->hasExtension<TerrainLocationExtension>()) {
                return true;
            }

            auto plane = mousePickTaggedEntity<PlaneComponent>(m_renderPanel, gaunlet::Prefab::RenderPipelineExtensions::EntitySelectionExtension::EntityLayer::SceneLayer);

            if (plane) {

                // Remember the plane
                m_plane = plane;

                m_terrainLocation = mousePickTerrainLocation(m_renderPanel);

                // If there isn't a stamp yet, add it
                if (!m_plane.hasComponent<StampComponent>()) {
                    m_plane.addComponent<StampComponent>(
                        glm::vec2(m_terrainLocation.x, m_terrainLocation.z),
                        m_stampUVSize,
                        m_stamp
                    );
                } else { // Otherwise, update it
                    auto& stamp = m_plane.getComponent<StampComponent>();
                    stamp.m_uvOrigin = {m_terrainLocation.x, m_terrainLocation.z};
                }

            } else { // If we're not hovering any plane, remove the stamp  from the last plane (if any), and forget the plane

                if (m_plane && m_plane.hasComponent<StampComponent>()) {
                    m_plane.removeComponent<StampComponent>();
                }

            }

            return true;

        }

        glm::vec4 mousePickTerrainLocation(gaunlet::Editor::RenderPanel* renderPanel) {

            unsigned int pixelPositionX = renderPanel->getMousePositionX() * gaunlet::Core::Window::getCurrentInstance()->getDPI();
            unsigned int pixelPositionY = renderPanel->getMousePositionYInverted() * gaunlet::Core::Window::getCurrentInstance()->getDPI();

            return renderPanel->getRenderPipeline()
                ->getExtension<TerrainLocationExtension>()
                ->mousePickTerrainLocation(pixelPositionX, pixelPositionY);

        }

    };

}