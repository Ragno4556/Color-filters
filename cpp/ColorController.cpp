 //So Windows doesn't conflict with min and max from the standard c++ library.
#define NOMINMAX

#include "ColorController.h"

#include <cmath>
#include <iostream>


//Calculate RGB values from a tint (hue) float (0-359) and an intensity float (0-1) based on HSL to RGB equations.
RGB ColorController::calculateRGB(const float tint, const float intensity){

    if(tint < 0 || tint >= 360){ //Check if tint is between 0 and 359
        std::cerr << "calculateRGB(): Tint value must be between 0 and 359\n";
        return RGB{1.0f, 1.0f, 1.0f};
    }

    //Variables used for calculation.
    float intermediateColorValue, r, g, b;
    intermediateColorValue =(1.0f - (std::abs(std::fmod(tint / 60.0f, 2.0f) - 1.0f) * 1.0f));

    //If statements for HSL to RGB equation based on hue degree (tint).
    if (tint >= 00.0f && tint < 60.0f) {
        r = 1.0f;
        g = intermediateColorValue;
        b = 0.0f;
    }
    else if (tint >= 60.0f && tint < 120.0f) {
        r = intermediateColorValue;
        g = 1.0f;
        b = 0.0f;
    }
    else if (tint >= 120.0f && tint < 180.0f) {
        r = 0.0f;
        g = 1.0f;
        b = intermediateColorValue;
    }
    else if (tint >= 180.0f && tint < 240.0f) {
        r = 0.0f;
        g = intermediateColorValue;
        b = 1.0f;
    }
    else if (tint >= 240.0f && tint < 300.0f) {
        r = intermediateColorValue;
        g = 0.0f;
        b = 1.0f;
    }
    else {
        r = 1.0f;
        g = 0.0f;
        b = intermediateColorValue;
    }

    //Blends between no filter and the selected tint according to the intensity slider.
    return RGB{ 1.0f + (r - 1.0f) * intensity, 1.0f + (g - 1.0f) * intensity, 1.0f + (b - 1.0f) * intensity};
}