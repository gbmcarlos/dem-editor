#pragma once

#include "dem-editor/pch.h"

namespace DemEditor {

    class Brush {

    public:

        virtual const char* getFragmentShaderPath() = 0;
        virtual const gaunlet::Core::Ref<gaunlet::Graphics::TextureImage2D>& getBrushStampTexture() = 0;

    };

}