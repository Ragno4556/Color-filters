#pragma once

#include "RGB.h"

//ColorController handles any external calculations involving colors.
class ColorController {
    public:
        static RGB calculateRGB(const float tint, const float intensity);
};