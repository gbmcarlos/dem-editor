#pragma once

#include <utility>

#include "terramorph/pch.h"

namespace terramorph {

    struct StampComponent {

        StampComponent(const glm::vec2& uvOrigin, float uvSize, gaunlet::Core::Ref<gaunlet::Graphics::Texture> stampTexture)
            : m_uvOrigin(uvOrigin), m_uvSize(uvSize), m_stampTexture(std::move(stampTexture)) {}

        glm::vec2 m_uvOrigin;
        float m_uvSize;
        gaunlet::Core::Ref<gaunlet::Graphics::Texture> m_stampTexture;

    };

}