#pragma once

#include <utility>

#include "terramorph/pch.h"

namespace terramorph::Core {

    struct StampComponent {

        StampComponent(const glm::vec2& origin, float width, float height, gaunlet::Core::Ref<gaunlet::Graphics::TextureImage2D> stampTexture)
            : m_origin(origin), m_width(width), m_height(height), m_stampTexture(std::move(stampTexture)) {}

        glm::vec2 m_origin;
        float m_width;
        float m_height;
        gaunlet::Core::Ref<gaunlet::Graphics::TextureImage2D> m_stampTexture;

    };

}