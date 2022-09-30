#pragma once

#include "gaunlet/prefab/gui-panels/EntityComponentsPanel.h"

namespace DemEditor {

    class EntityComponentsPanel : public gaunlet::Prefab::GuiPanels::EntityComponentsPanel {

    protected:

        void sections(gaunlet::Scene::Entity entity) override {

            gaunlet::Prefab::GuiPanels::EntityComponentsPanel::sections(entity);

            if (entity.hasComponent<PlaneComponent>()) {
                terrainComponentProperties(entity.getComponent<PlaneComponent>());
            }

        }

    private:

        void terrainComponentProperties(PlaneComponent& terrain) {

            if (ImGui::CollapsingHeader("Terrain Component")) {
                ImGui::DragFloat("Plane Size: ", &terrain.m_size, m_sliderSpeed);
                ImGui::DragFloat("Target Resolution: ", &terrain.m_targetResolution, m_sliderSpeed);
                ImGui::DragFloat("Resolution Slope: ", &terrain.m_resolutionSlope, m_sliderSpeed);
                ImGui::DragFloat("Triangle Size: ", &terrain.m_triangleSize, m_sliderSpeed);
                ImGui::DragFloat("Max Height: ", &terrain.m_maxHeight, m_sliderSpeed);
            }

        }

    };

}