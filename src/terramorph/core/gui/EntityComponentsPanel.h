#pragma once

#include "gaunlet/prefab/gui-panels/EntityComponentsPanel.h"

namespace terramorph::Core {

    class EntityComponentsPanel : public gaunlet::Prefab::GuiPanels::EntityComponentsPanel {

    protected:

        void sections(gaunlet::Scene::Entity entity) override {

            gaunlet::Prefab::GuiPanels::EntityComponentsPanel::sections(entity);

            if (entity.hasComponent<TerrainComponent>()) {
                terrainComponentProperties(entity.getComponent<TerrainComponent>());
            }

        }

    private:

        void terrainComponentProperties(TerrainComponent& terrain) {

            if (ImGui::CollapsingHeader("Terrain Component")) {
                ImGui::Image(
                    (void *)(intptr_t)terrain.getHeightmap()->getRendererId(),
                    ImVec2(200, 200),
                    ImVec2(0, 1), ImVec2(1, 0)
                );
                ImGui::DragFloat("Max Height: ", &terrain.m_maxHeight, m_sliderSpeed);
                ImGui::DragFloat("Triangle Size: ", &terrain.m_triangleSize, m_sliderSpeed);
                ImGui::DragFloat("Target Quad Resolution: ", &terrain.m_targetQuadResolution, m_sliderSpeed);
                ImGui::DragFloat("Quad Resolution Slope: ", &terrain.m_quadResolutionSlope, m_sliderSpeed);

                if (ImGui::Button("Reset Heightmap")) {
                    terrain.resetHeightmap();
                }

            }

        }

    };

}