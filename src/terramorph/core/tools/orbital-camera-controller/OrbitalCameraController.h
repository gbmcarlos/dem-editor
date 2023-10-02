#pragma once

#include "gaunlet/prefab/editor-tools/first-person-camera-controller/FirstPersonCameraController.h"

namespace terramorph::Core {

    class OrbitalCameraController : public gaunlet::Prefab::EditorTools::FirstPersonCameraController {

    public:

        explicit OrbitalCameraController(const char* renderPanelId);

        const char* getName() override;
        void onGuiRender() override;
        void onUpdate(gaunlet::Core::TimeStep timeStep) override;

    };

}