#pragma once

#include "terramorph/pch.h"

namespace terramorph::Core {

    class DeformationBrush {

    public:

        virtual const gaunlet::Core::Ref<gaunlet::Graphics::Texture>& getBrushStampTexture() = 0;
        virtual void onUpdate(gaunlet::Core::TimeStep timeStep) {}
        virtual void onGuiRender() {}

    };

}