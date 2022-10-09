#pragma once

#include "gaunlet/prefab/editor-tools/EntitySelectionTool.h"
#include "terramorph/core/graphics/render-pipeline/extensions/TerrainLocationExtension.h"

namespace terramorph::Core {

    class TerrainSelectionTool : public gaunlet::Prefab::EditorTools::EntitySelectionTool {

    protected:

        bool isHoveringTerrain() {

            auto renderPanel = getWorkspace()->getHoveredRenderPanel();
            if (!renderPanel || !renderPanel->getRenderPipeline()->hasExtension<TerrainLocationExtension>()) {
                return false;
            }

            auto plane = mousePickTaggedEntity<TerrainComponent>(renderPanel, gaunlet::Prefab::RenderPipelineExtensions::EntitySelectionExtension::EntityLayer::SceneLayer);
            if (!plane) {
                return false;
            }

            return true;

        }

        void mousePickTerrain() {

            selectHoveredRenderPanelWithExtension<TerrainLocationExtension>();

            if (!m_renderPanel) {
                m_terrain = {};
                return;
            }

            m_terrain = mousePickTaggedEntity<TerrainComponent>(
                m_renderPanel,
                gaunlet::Prefab::RenderPipelineExtensions::EntitySelectionExtension::EntityLayer::SceneLayer
            );

        }

        void mousePickTerrainLocation() {

            mousePickTerrain();

            if (!m_renderPanel || !m_terrain) {
                return;
            }

            auto& terrainComponent = m_terrain.getComponent<TerrainComponent>();

            unsigned int pixelPositionX = m_renderPanel->getMousePositionX() * gaunlet::Core::Window::getCurrentInstance()->getDPI();
            unsigned int pixelPositionY = m_renderPanel->getMousePositionYInverted() * gaunlet::Core::Window::getCurrentInstance()->getDPI();

            auto terrainLocation = m_renderPanel->getRenderPipeline()
                ->getExtension<TerrainLocationExtension>()
                ->mousePickTerrainLocation(pixelPositionX, pixelPositionY);

            m_terrainLocation = terrainComponent.uvLocation2WorldCoordinates(terrainLocation);

        }

        gaunlet::Scene::Entity m_terrain;
        glm::vec3 m_terrainLocation;

    };

}