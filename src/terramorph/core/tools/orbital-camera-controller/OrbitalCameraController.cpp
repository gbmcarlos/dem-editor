#include "terramorph/core/tools/orbital-camera-controller/OrbitalCameraController.h"

namespace terramorph::Core {

    OrbitalCameraController::OrbitalCameraController(const char* renderPanelId)
        : FirstPersonCameraController(renderPanelId) {}

    const char* OrbitalCameraController::getName() {
        return "Orbital Camera Controller";
    }

    void OrbitalCameraController::onGuiRender() {}

    void OrbitalCameraController::onUpdate(gaunlet::Core::TimeStep timeStep) {

        selectRenderPanel();
        if (m_renderPanel == nullptr) {
            return;
        }

        auto& camera = m_renderPanel->getCamera();
        float rotation = timeStep * m_rotationSensitivity;

        // ROTATION
        if (m_rotating) {

            bool shiftPressed = gaunlet::Core::Input::isKeyPressed(GL_KEY_LEFT_SHIFT) || gaunlet::Core::Input::isKeyPressed(GL_KEY_RIGHT_SHIFT);

            glm::mat4 rotationMatrix;

            if (shiftPressed) {

                camera->rotate({camera->getForward(), m_rotationDelta.x * rotation});

            } else {
                glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(m_rotationDelta.x * rotation), camera->getUp());
                glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(m_rotationDelta.y * rotation), camera->getRight());
                glm::vec3 cameraPosition = glm::vec3(glm::vec4(camera->getPosition(), 1.0f) * rotationX * rotationY);
                camera->setPosition(cameraPosition);
                camera->lookAt({0, 0, 0});
            }

        }

    }

}