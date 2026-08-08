#include "ColorController.h"

#include <cmath>
#include <iostream>

RGB ColorController::calculateRGB(float tint, float intensity)
{
    if (tint < 0.0f || tint >= 360.0f)
    {
        std::cerr << "ColorController::calculateRGB: "
                  << "Tint must be between 0 and 359.\n";
        return {1.0f, 1.0f, 1.0f};
    }

    const float intermediate = 1.0f - std::abs(std::fmod(tint / 60.0f, 2.0f) - 1.0f);

    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;

    if (tint < 60.0f)
    {
        red = 1.0f;
        green = intermediate;
    }
    else if (tint < 120.0f)
    {
        red = intermediate;
        green = 1.0f;
    }
    else if (tint < 180.0f)
    {
        green = 1.0f;
        blue = intermediate;
    }
    else if (tint < 240.0f)
    {
        green = intermediate;
        blue = 1.0f;
    }
    else if (tint < 300.0f)
    {
        red = intermediate;
        blue = 1.0f;
    }
    else
    {
        red = 1.0f;
        blue = intermediate;
    }

    return {1.0f + (red - 1.0f) * intensity, 1.0f + (green - 1.0f) * intensity, 1.0f + (blue - 1.0f) * intensity};
}
