#pragma once

#include "terramorph/pch.h"

namespace terramorph {

    class Brush {

    public:

        virtual const char* getFragmentShaderPath() = 0;
        virtual const gaunlet::Core::Ref<gaunlet::Graphics::TextureImage2D>& getBrushStampTexture() = 0;

    };

}