#pragma once

#include "gaunlet/prefab/editor-tools/EntitySelectionTool.h"
#include "terramorph/core/graphics/render-pipeline/extensions/PlanetLocationExtension.h"
#include "terramorph/core/graphics/components/PlanetComponent.h"

namespace terramorph::Core {

    class PlanetSelectionTool : public gaunlet::Prefab::EditorTools::EntitySelectionTool {

    protected:

        bool isHoveringPlanet() {

            auto renderPanel = getWorkspace()->getHoveredRenderPanel();
            if (!renderPanel || !renderPanel->getRenderPipeline()->hasExtension<PlanetLocationExtension>()) {
                return false;
            }

            auto planet = mousePickTaggedEntity<PlanetComponent>(renderPanel, gaunlet::Prefab::RenderPipelineExtensions::EntitySelectionExtension::EntityLayer::SceneLayer);
            if (!planet) {
                return false;
            }

            return true;

        }

        void mousePickPlanet() {

            selectHoveredRenderPanelWithExtension<PlanetLocationExtension>();

            if (!m_renderPanel) {
                m_planet = {};
                return;
            }

            m_planet = mousePickTaggedEntity<PlanetComponent>(
                m_renderPanel,
                gaunlet::Prefab::RenderPipelineExtensions::EntitySelectionExtension::EntityLayer::SceneLayer
            );

        }

        void mousePickPlanetLocation() {

            mousePickPlanet();

            if (!m_renderPanel || !m_planet) {
                return;
            }

            auto& planetComponent = m_planet.getComponent<PlanetComponent>();

            unsigned int pixelPositionX = m_renderPanel->getMousePositionX() * gaunlet::Core::Window::getCurrentInstance()->getDPI();
            unsigned int pixelPositionY = m_renderPanel->getMousePositionYInverted() * gaunlet::Core::Window::getCurrentInstance()->getDPI();

            m_planetLocation = m_renderPanel->getRenderPipeline()
                ->getExtension<PlanetLocationExtension>()
                ->mousePickPlanetLocation(pixelPositionX, pixelPositionY);

        }

        gaunlet::Scene::Entity m_planet;
        glm::vec2 m_planetLocation;

    };

}