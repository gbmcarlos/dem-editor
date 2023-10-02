#pragma once

#include "gaunlet/prefab/gui-panels/EntityComponentsPanel.h"
#include "terramorph/core/graphics/components/PlanetComponent.h"
#include "terramorph/core/graphics/components/TerrainComponent.h"

namespace terramorph::Core {

    class EntityComponentsPanel : public gaunlet::Prefab::GuiPanels::EntityComponentsPanel {

    protected:

        void sections(gaunlet::Scene::Entity entity) override {

            gaunlet::Prefab::GuiPanels::EntityComponentsPanel::sections(entity);

            if (entity.hasComponent<PlanetComponent>()) {
                planetComponentProperties(entity.getComponent<PlanetComponent>());
            }

            if (entity.hasComponent<TerrainComponent>()) {
                terrainComponentProperties(entity.getComponent<TerrainComponent>());
            }

        }

    private:

        void planetComponentProperties(PlanetComponent& planet) {

            if (ImGui::CollapsingHeader("Planet Component")) {
                ImGui::DragFloat("Radius: ", &planet.m_radius, m_sliderSpeed);
                ImGui::SliderFloat("Triangle Size: ", &planet.m_triangleSize, 0.0f, 200.0f);
                ImGui::DragFloat("Target resolution: ", &planet.m_targetResolution, m_sliderSpeed);
                ImGui::DragFloat("Resolution slope: ", &planet.m_resolutionSlope, m_sliderSpeed);
                ImGui::SliderFloat("Coverage: ", &planet.m_coverage, 0.0f, 90.0f);
            }

        }

        void terrainComponentProperties(TerrainComponent& terrain) {

            if (ImGui::CollapsingHeader("Terrain Component")) {
                ImGui::Image(
                    (void *)(intptr_t)terrain.getHeightmap()->getRendererId(),
                    ImVec2(200, 200),
                    ImVec2(0, 1), ImVec2(1, 0)
                );
                ImGui::DragFloat("Max Height: ", &terrain.m_maxHeight, m_sliderSpeed);

                if (ImGui::Button("Reset Heightmap")) {
                    terrain.resetHeightmap();
                }

            }

        }

    };

}