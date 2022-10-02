#pragma once

#include "gaunlet/core/event/events/MouseEvent.h"
#include "dem-editor/tools/TerrainSelectionTool.h"
#include "dem-editor/tools/terrain-deformation/Brush.h"

namespace DemEditor {

    class TerrainElevationTool : public TerrainSelectionTool {

    public:

        TerrainElevationTool() = default;

        const char* getName() override {
            return "Terrain Elevation";
        }

        const gaunlet::Core::Ref<Brush>& addBrush(const char* id, const gaunlet::Core::Ref<Brush>& brush) {

            m_brushes[id] = brush;

            std::map<gaunlet::Core::ShaderType, std::string> editorSources {
                {gaunlet::Core::ShaderType::Vertex, GL_PREFABS_PATH"/shaders/pass-through-vertex.glsl"},
                {gaunlet::Core::ShaderType::Fragment, brush->getFragmentShaderPath()}
            };

            std::string brushShaderId("brush-");
            brushShaderId += id;

            auto brushShader = m_shaderLibrary.load(brushShaderId, editorSources);
            brushShader->setUniform1i("heightmap", 0);

            return brush;
        }

        void activateBrush(const char* id) {
            m_activeBrushId = id;
        }

        const gaunlet::Core::Ref<Brush>& getActiveBrush() {

            auto iterator = m_brushes.find(m_activeBrushId);

            if (iterator == m_brushes.end()) {
                throw std::runtime_error("Brush not found");
            }

            return iterator->second;

        }

        bool onEvent(gaunlet::Core::Event &event) override {

            gaunlet::Core::EventDispatcher dispatcher(event);
            dispatcher.dispatch<gaunlet::Core::CursorMoveEvent>(GL_BIND_CALLBACK_FN(TerrainElevationTool::onCursorMoveEvent));

            return true;

        }

        void onUpdate(gaunlet::Core::TimeStep timeStep) override {

            if (!gaunlet::Core::Input::isMouseButtonPressed(0)) {
                return;
            }

            auto terrain = mousePickTerrain();
            if (!terrain) {
                return;
            }

            auto terrainLocation = mousePickTerrainLocation(m_renderPanel);
            auto& heightmap = terrain.getComponent<HeightmapComponent>();

            if (m_activeBrushId != nullptr) {

                // First make sure the stamp is update to the right position, since the terrain might have been deformed in the previous frame
                if (terrain.hasComponent<StampComponent>()) {
                    auto& stamp = terrain.getComponent<StampComponent>();
                    stamp.m_uvOrigin = {terrainLocation.x, terrainLocation.z};
                }

                // Then deform the terrain
                std::string brushShaderId("brush-");
                brushShaderId += m_activeBrushId;

                auto brushShader = m_shaderLibrary.get(brushShaderId.c_str());
                brushShader->setUniform2f("u_brushOrigin", {terrainLocation.x, terrainLocation.z});
                brushShader->setUniform1f("u_brushSize", m_brushSize);
                brushShader->setUniform1f("u_timeStep", timeStep);
                brushShader->setUniform1f("u_strength", m_strength);

                heightmap.update(brushShader);

            }

        }

    protected:

        gaunlet::Scene::Entity m_terrain;
        const char* m_activeBrushId = nullptr;
        std::unordered_map<const char*, gaunlet::Core::Ref<Brush>> m_brushes;
        gaunlet::Graphics::ShaderLibrary m_shaderLibrary;
        float m_brushSize = 0.05;
        float m_strength = 1.0f;

        bool onCursorMoveEvent(gaunlet::Core::CursorMoveEvent& event) {

            if (!m_activeBrushId) {
                return true;
            }

            if (isHoveringTerrain()) {

                m_terrain = mousePickTerrain(); // Remember the terrain
                auto terrainLocation = mousePickTerrainLocation(m_renderPanel);

                // If there isn't a stamp yet, add it
                if (!m_terrain.hasComponent<StampComponent>()) {
                    m_terrain.addComponent<StampComponent>(
                        glm::vec2(terrainLocation.x, terrainLocation.z),
                        m_brushSize,
                        getActiveBrush()->getBrushStampTexture()
                    );
                } else { // Otherwise, update it
                    auto& stamp = m_terrain.getComponent<StampComponent>();
                    stamp.m_uvOrigin = {terrainLocation.x, terrainLocation.z};
                    stamp.m_stampTexture = getActiveBrush()->getBrushStampTexture();
                }

            } else if (m_terrain && m_terrain.hasComponent<StampComponent>()) { // Remove ths stamp and forget the terrain entity
                m_terrain.removeComponent<StampComponent>();
                m_terrain = {};
            }

            return true;

        }

    };

}