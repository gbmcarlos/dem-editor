#pragma once

#include "terramorph/core/graphics/render-pipeline/TerrainRenderPipeline.h"
#include "terramorph/core/gui/EntityComponentsPanel.h"
#include "terramorph/core/tools/terrain-deformation/stamp-deformation/StampDeformationTool.h"
#include "terramorph/core/graphics/terrain-components/TerrainComponent.h"
#include "terramorph/core/tools/terrain-deformation/stamp-deformation/StampDeformationTool.h"
#include "terramorph/sculpting/stamps/procedural-radial-stamp/ProceduralRadialStamp.h"

#include "pch.h"

namespace terramorph {

    class Application : public gaunlet::Core::Application {

    public:

        Application() : gaunlet::Core::Application() {}

        void onReady() override {

            m_workspace = new gaunlet::Editor::Workspace();

            prepareLayout();
            prepareTools();
            prepareScene();

        }

        void onUpdate(gaunlet::Core::TimeStep timeStep) override {
            m_workspace->update(timeStep);
        }

        void onGuiRender() override {
            m_workspace->render();
        }

        void onEvent(gaunlet::Core::Event &event) override {
            m_workspace->onEvent(event);
        }

    private:
        gaunlet::Editor::Workspace* m_workspace = nullptr;

        void prepareLayout() {

            auto window = gaunlet::Core::Window::getCurrentInstance();
            unsigned int viewportWidth = window->getViewportWidth();
            unsigned int viewportHeight = window->getViewportHeight();

            // Set the docking layout
            m_workspace->setLayoutSpec({
                {
                    {gaunlet::Editor::DockSpacePosition::Left, 0.2f, {"Render Panel"}},
                    {gaunlet::Editor::DockSpacePosition::Down, 0.3f,  0, {"Entity Components"}},
                    {gaunlet::Editor::DockSpacePosition::Down, 0.7f,  0, {"Tools Manager"}},
                    {gaunlet::Editor::DockSpacePosition::Center, 0.0f,  {"Scene"}, ImGuiDockNodeFlags_NoTabBar}
                }, viewportWidth, viewportHeight
            });

            // Push the GUI panels
            m_workspace->pushPanel("render-panel", new gaunlet::Prefab::GuiPanels::RenderPanelComponentsPanel, "Render Panel");
            m_workspace->pushPanel("entity-components", new terramorph::Core::EntityComponentsPanel, "Entity Components");
            m_workspace->pushPanel("tools", new gaunlet::Prefab::GuiPanels::ToolsManagerPanel, "Tools Manager");

            // Prepare the Render Panel
            m_workspace->addRenderPipeline("main", gaunlet::Core::CreateRef<Core::TerrainRenderPipeline>(
                "terrain",
                gaunlet::Core::CreateRef<gaunlet::Scene::DirectionalLightComponent>(
                    glm::vec3(0.8f, 0.8f, 0.8f),
                    glm::vec3(-0.2f, -1.0f, -0.3f),
                    0.2f, 0.7f
                ),
                gaunlet::Core::CreateRef<gaunlet::Scene::SkyboxComponent>(gaunlet::Core::CreateRef<gaunlet::Prefab::Skyboxes::SimpleSkyboxCubeMap>())
            ));
            m_workspace->pushPanel(
                "main",
                new gaunlet::Editor::RenderPanel(),
                "Scene",
                "main",
                "main",
                "main"
            );

        }

        void prepareTools() {

            auto stampDeformationTool = gaunlet::Core::CreateRef<Core::StampDeformationTool>(5.0f);
            stampDeformationTool->addBrush("radial-elevation", gaunlet::Core::CreateRef<Sculpting::ProceduralRadialStamp>());
            stampDeformationTool->activateBrush("radial-elevation");

            m_workspace->addTool("fp-camera-controller", gaunlet::Core::CreateRef<gaunlet::Prefab::EditorTools::FirstPersonCameraController>("main", 20.0f, 0.5f));
            m_workspace->addTool("transformer", gaunlet::Core::CreateRef<gaunlet::Prefab::EditorTools::TransformerTool>());
            m_workspace->addTool("stamp-deformation", stampDeformationTool);
            m_workspace->activateTool("stamp-deformation");

        }

        void prepareScene() {

            auto window = gaunlet::Core::Window::getCurrentInstance();
            unsigned int viewportWidth = window->getViewportWidth();
            unsigned int viewportHeight = window->getViewportHeight();

            // Scene components
            auto mainCamera = gaunlet::Core::CreateRef<gaunlet::Scene::PerspectiveCamera>(45.0f, (float) viewportWidth / (float) viewportHeight, 1.0f, -100000.0f);
            m_workspace->addCamera("main", mainCamera);
            mainCamera->setPosition({0.0f, 40.0f, 40.0f});
            mainCamera->setRotation(-90.0f, -50.0f);

            m_workspace->addScene("main", gaunlet::Core::CreateRef<gaunlet::Scene::Scene>());

            // Scene entities
            auto& mainScene = m_workspace->getScene("main");

            auto terrain = mainScene->createTaggedEntity<gaunlet::Editor::SceneEntityTag>("terrain");
            terrain.addComponent<Core::TerrainComponent>(
                100.0f, 40.0f, 10.0f, // Dimensions
                1.0f, 25.0f, // Resolution
                mainCamera,
                5.0f, 0.5f // Quad subdivision
            );

        }

    };

}