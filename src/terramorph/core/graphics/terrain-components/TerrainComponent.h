#pragma once

#include "gaunlet/scene/camera/PerspectiveCamera.h"
#include "terramorph/core/graphics/procedural-plane/ProceduralPlane.h"
#include "gaunlet/graphics/shader/Shader.h"
#include "gaunlet/graphics/texture/Texture.h"
#include "gaunlet/graphics/framebuffer/Framebuffer.h"

namespace terramorph::Core {

    class TerrainComponent {

    public:

        TerrainComponent(float meshWidth, float meshDepth, float maxHeight, float heightmapResolution, float triangleSize, gaunlet::Core::Ref<gaunlet::Scene::PerspectiveCamera> camera, float quadTargetResolution, float quadResolutionSlope);

        std::vector<PlaneQuad> getMeshContent();

        float getHeightmapResolution();
        float getMeshWidth();
        float getMeshDepth();

        const gaunlet::Core::Ref<gaunlet::Scene::PerspectiveCamera>& getCamera();
        const gaunlet::Core::Ref<gaunlet::Graphics::Texture>& getHeightmap();

        void updateHeightmap(const gaunlet::Core::Ref<gaunlet::Graphics::Shader>& shader);
        void updateHeightmap(const gaunlet::Core::Ref<gaunlet::Graphics::Shader>& shader, glm::vec2 origin, float width, float height);
        void resetHeightmap();

        glm::vec3 uvLocation2WorldCoordinates(glm::vec3 location) const;
        glm::vec2 uvLocation2WorldCoordinates(glm::vec2 location) const;
        float uvWidth2WorldCoordinates(float width) const;
        float uvHeight2WorldCoordinates(float height) const;
        float uvDepth2WorldCoordinates(float depth) const;

        glm::vec3 worldLocation2UVCoordinates(glm::vec3 location) const;
        glm::vec2 worldLocation2UVCoordinates(glm::vec2 location) const;
        float worldWidth2UVCoordinates(float width) const;
        float worldHeight2UVCoordinates(float height) const;
        float worldDepth2UVCoordinates(float depth) const;

        float m_maxHeight;
        float m_triangleSize;
        float m_targetQuadResolution;
        float m_quadResolutionSlope;

    private:

        void swapHeightmap(glm::vec2 uvOrigin, float uvWidth, float uvHeight);
        void createHeightmap();

        float m_meshWidth;
        float m_meshDepth;
        float m_heightmapResolution;
        gaunlet::Core::Ref<gaunlet::Scene::PerspectiveCamera> m_camera;
        gaunlet::Core::Ref<gaunlet::Graphics::Texture> m_heightmap;

        gaunlet::Core::Ref<gaunlet::Graphics::Framebuffer> m_heightmapFramebuffer;

    };

}