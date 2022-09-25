#pragma once

#include "gaunlet/prefab/editor-tools/EntitySelectionTool.h"
#include "dem-editor/graphics/components/TerrainComponents.h"
#include "gaunlet/core/event/events/MouseEvent.h"

namespace DemEditor {

    class TerrainSelectionTool : public gaunlet::Prefab::EditorTools::EntitySelectionTool {

    public:

        const char* getName() override {
            return "Terrain Location";
        }

        bool onEvent(gaunlet::Core::Event &event) override {

            gaunlet::Core::EventDispatcher dispatcher(event);
            dispatcher.dispatch<gaunlet::Core::MouseButtonPress>(GL_BIND_CALLBACK_FN(TerrainSelectionTool::onMousePressEvent));
            dispatcher.dispatch<gaunlet::Core::MouseButtonRelease>(GL_BIND_CALLBACK_FN(TerrainSelectionTool::onMouseReleaseEvent));

            return true;

        }

        void onGuiRender() override {

            if (m_clicking) {
                ImGui::Text("Terrain Location: (%f %f %f)", m_terrainLocation.x, m_terrainLocation.y, m_terrainLocation.z);
            }

        }

    protected:

        bool m_clicking = false;
        glm::vec4 m_terrainLocation;

        bool onMousePressEvent(gaunlet::Core::MouseButtonPress& event) {

            auto renderPanel = getWorkspace()->getHoveredRenderPanel();
            if (!renderPanel) {
                return true;
            }

            auto terrainEntity = mousePickTaggedEntity<TerrainComponent>(renderPanel, 1);

            if (!terrainEntity) {
                return true;
            }

            m_clicking = true;

            auto terrainLocation = mousePickTerrainLocation(renderPanel, terrainEntity.getComponent<TerrainComponent>());

            m_terrainLocation = terrainLocation;

            return true;

        }

        bool onMouseReleaseEvent(gaunlet::Core::MouseButtonRelease& event) {
            m_clicking = false;
            return true;
        }

        glm::vec4 mousePickTerrainLocation(gaunlet::Editor::RenderPanel* renderPanel, TerrainComponent& terrain) {

            unsigned int pixelPositionX = renderPanel->getMousePositionX() * gaunlet::Core::Window::getCurrentInstance()->getDPI();
            unsigned int pixelPositionY = renderPanel->getMousePositionYInverted() * gaunlet::Core::Window::getCurrentInstance()->getDPI();

            auto terrainLocation = getWorkspace()->getRenderPipeline(renderPanel->getRenderPipelineId())->getFramebuffer()->readPixel<glm::vec4>(
                3,
                pixelPositionX,
                pixelPositionY
            );

            float halfSize = terrain.m_size/2;

            // XZ plane position from uv coordinates
            float worldX = (terrainLocation.x * terrain.m_size) - halfSize;
            float worldZ = -((terrainLocation.z * terrain.m_size) - halfSize);

            // Y position based from max height
            float worldY = terrainLocation.y * terrain.m_maxHeight;

            return {worldX, worldY, worldZ, 0.0f};

        }

    };

}