#pragma once

#include "gaunlet/core/event/events/MouseEvent.h"
#include "terramorph/core/tools/PlanetSelectionTool.h"
#include "terramorph/core/tools/terrain-deformation/stamp-deformation/DeformationBrush.h"
#include "terramorph/core/graphics/components/StampComponent.h"

namespace terramorph::Core {

    class StampDeformationTool : public PlanetSelectionTool {

    public:

        StampDeformationTool(float defaultStampSize = 0.05f)
            : m_stampSize(defaultStampSize) {
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

            mousePickPlanet();
            if (!m_planet) {
                return;
            }

            if (m_activeBrushId != nullptr) {

                auto& planetComponent = m_planet.getComponent<PlanetComponent>();
                auto& terrainComponent = m_planet.getComponent<TerrainComponent>();

                auto& shader = m_shaderLibrary.get("main");
                shader->setUniform2f("u_stampUVOrigin", m_planetLocation);
                shader->setUniform1f("u_stampUVWidth", m_stampSize);
                shader->setUniform1f("u_stampUVHeight", m_stampSize);
                shader->setUniform1f("u_timeStep", (float) timeStep);
                shader->setUniform1f("u_strength", m_strength);

                getActiveBrush()->getBrushStampTexture()->activate(1);
                terrainComponent.updateHeightmap(shader, m_planetLocation, m_stampSize, m_stampSize);

            }

        }

        void onGuiRender() override {

            if (m_planet) {
                auto& planetComponent = m_planet.getComponent<PlanetComponent>();
                auto lonLat = planetComponent.planeUV2LonLat(m_planetLocation.x, m_planetLocation.y);
                ImGui::Text("Terrain Location: (%f %f)", m_planetLocation.x, m_planetLocation.y);
            } else {
                ImGui::Text("Terrain Location:");
            }

            ImGui::SliderFloat("Stamp Size", &m_stampSize, 0.01f, 0.5f);
            ImGui::SliderFloat("DeformationStrength", &m_strength, 0.0f, 1.0f);

            if (m_activeBrushId) {
                if (ImGui::CollapsingHeader("Brush properties")) {
                    getActiveBrush()->onGuiRender();
                }
            }

        }

    protected:

        const char* m_activeBrushId = nullptr;
        std::unordered_map<const char*, gaunlet::Core::Ref<DeformationBrush>> m_brushes;
        gaunlet::Graphics::ShaderLibrary m_shaderLibrary;
        float m_stampSize = 5.0f;
        float m_strength = 1.0f;

        bool onCursorMoveEvent(gaunlet::Core::CursorMoveEvent& event) {

            if (!m_activeBrushId) {
                return true;
            }

            if (isHoveringPlanet()) {

                // Sets m_planet and m_planetLocation
                mousePickPlanetLocation();

                // If there isn't a stamp yet, add it
                if (!m_planet.hasComponent<StampComponent>()) {

                    m_planet.addComponent<StampComponent>(
                        m_planetLocation,
                        m_stampSize, m_stampSize,
                        getActiveBrush()->getBrushStampTexture()
                    );

                } else { // Otherwise, update it

                    auto& stamp = m_planet.getComponent<StampComponent>();
                    stamp.m_origin = m_planetLocation;
                    stamp.m_stampTexture = getActiveBrush()->getBrushStampTexture();

                }

            } else if (m_planet && m_planet.hasComponent<StampComponent>()) { // Remove ths stamp and forget the terrain entity
                m_planet.removeComponent<StampComponent>();
                m_planet = {};
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