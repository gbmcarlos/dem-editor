#pragma once

#include "gaunlet/core/event/events/MouseEvent.h"
#include "terramorph/core/tools/TerrainSelectionTool.h"
#include "terramorph/core/tools/terrain-deformation/stamp-deformation/DeformationBrush.h"
#include "terramorph/core/graphics/terrain-components/HeightmapComponent.h"
#include "terramorph/core/graphics/terrain-components/StampComponent.h"

namespace terramorph::Core {

    class StampDeformationTool : public TerrainSelectionTool {

    public:

        StampDeformationTool() {
            loadShaders();
        }

        const char* getName() override {
            return "Stamp Deformation";
        }

        const gaunlet::Core::Ref<DeformationBrush>& addBrush(const char* id, const gaunlet::Core::Ref<DeformationBrush>& brush) {
            m_brushes[id] = brush;
            return brush;
        }

        void activateBrush(const char* id) {
            m_activeBrushId = id;
        }

        const gaunlet::Core::Ref<DeformationBrush>& getActiveBrush() {

            auto iterator = m_brushes.find(m_activeBrushId);

            if (iterator == m_brushes.end()) {
                throw std::runtime_error("Brush not found");
            }

            return iterator->second;

        }

        bool onEvent(gaunlet::Core::Event &event) override {

            gaunlet::Core::EventDispatcher dispatcher(event);
            dispatcher.dispatch<gaunlet::Core::CursorMoveEvent>(GL_BIND_CALLBACK_FN(StampDeformationTool::onCursorMoveEvent));

            return true;

        }

        void onUpdate(gaunlet::Core::TimeStep timeStep) override {

            if (m_activeBrushId) {
                getActiveBrush()->onUpdate(timeStep);
            }

            if (!gaunlet::Core::Input::isMouseButtonPressed(0)) {
                return;
            }

            auto terrain = mousePickTerrain();
            if (!terrain) {
                return;
            }

            auto& heightmap = terrain.getComponent<HeightmapComponent>();

            if (m_activeBrushId != nullptr) {

                auto& shader = m_shaderLibrary.get("main");
                shader->setUniform2f("u_stampOrigin", {m_terrainLocation.x, m_terrainLocation.z});
                shader->setUniform1f("u_stampSize", m_stampSize);
                shader->setUniform1f("u_timeStep", (float) timeStep);
                shader->setUniform1f("u_strength", m_strength);

                getActiveBrush()->getBrushStampTexture()->activate(1);
                heightmap.updateQuad(shader, {m_terrainLocation.x, m_terrainLocation.z}, m_stampSize);

            }

        }

        void onGuiRender() override {

            ImGui::SliderFloat("Stamp Size", &m_stampSize, 0.001f, 0.5f);
            ImGui::SliderFloat("DeformationStrength", &m_strength, 0.0f, 1.0f);

            if (m_activeBrushId) {
                if (ImGui::CollapsingHeader("Brush properties")) {
                    getActiveBrush()->onGuiRender();
                }
            }

        }

    protected:

        gaunlet::Scene::Entity m_terrain;
        glm::vec3 m_terrainLocation;
        const char* m_activeBrushId = nullptr;
        std::unordered_map<const char*, gaunlet::Core::Ref<DeformationBrush>> m_brushes;
        gaunlet::Graphics::ShaderLibrary m_shaderLibrary;
        float m_stampSize = 0.05;
        float m_strength = 1.0f;

        bool onCursorMoveEvent(gaunlet::Core::CursorMoveEvent& event) {

            if (!m_activeBrushId) {
                return true;
            }

            if (isHoveringTerrain()) {

                m_terrain = mousePickTerrain(); // Remember the terrain
                m_terrainLocation = mousePickTerrainLocation(m_renderPanel);

                // If there isn't a stamp yet, add it
                if (!m_terrain.hasComponent<StampComponent>()) {
                    m_terrain.addComponent<StampComponent>(
                        glm::vec2(m_terrainLocation.x, m_terrainLocation.z),
                        m_stampSize,
                        getActiveBrush()->getBrushStampTexture()
                    );
                } else { // Otherwise, update it
                    auto& stamp = m_terrain.getComponent<StampComponent>();
                    stamp.m_uvOrigin = {m_terrainLocation.x, m_terrainLocation.z};
                    stamp.m_stampTexture = getActiveBrush()->getBrushStampTexture();
                }

            } else if (m_terrain && m_terrain.hasComponent<StampComponent>()) { // Remove ths stamp and forget the terrain entity
                m_terrain.removeComponent<StampComponent>();
                m_terrain = {};
            }

            return true;

        }

    private:

        void loadShaders() {

            std::map<gaunlet::Core::ShaderType, std::string> sources {
                {gaunlet::Core::ShaderType::Vertex, WORKING_DIR"/core/tools/terrain-deformation/stamp-deformation/shaders/vertex.glsl"},
                {gaunlet::Core::ShaderType::Fragment, WORKING_DIR"/core/tools/terrain-deformation/stamp-deformation/shaders/fragment.glsl"}
            };

            auto shader = m_shaderLibrary.load("main", sources);
            shader->setUniform1i("heightmap", 0);
            shader->setUniform1i("stamp", 1);

        }

    };

}