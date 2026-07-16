#define UNICODE
#define _UNICODE
#define NOMINMAX

#include "Converter.h"
#include <iostream>
#include <algorithm>
#include <cmath>

std::array<HSL, 256> Converter::RGBToHSL(std::array<std::array<WORD, 256>, 3> WordArray)
{
    // Create return HSL array and double RGB array
    std::array<HSL, 256> returnArray;
    std::array<std::array<double, 256>, 3> doubleArray;

    // Fill RGB array with normalized float values
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 256; j++)
        {
            doubleArray[i][j] = static_cast<double>(WordArray[i][j]) / 65535.0;
        }
    }

    for (int i = 0; i < 256; i++)
    {

        double r, g, b, maxVal, minVal, delta, luminance, saturation, hue;

        // Obtain RGB values for current index
        r = doubleArray[0][i];
        g = doubleArray[1][i];
        b = doubleArray[2][i];

        // Find the strongest and weakest channels (min/max)
        maxVal = std::max({r, g, b});
        minVal = std::min({r, g, b});

        // Define delta (chroma)
        delta = maxVal - minVal;

        // Calculate luminance(lightness)
        luminance = (maxVal + minVal) / 2.0;

        // Hue and saturation 0 cases
        if (delta == 0)
        {
            hue = 0;
            saturation = 0;
        }
        else
        {
            // Calculate saturation
            saturation = (delta) / (1 - (std::abs(2 * luminance) - 1));

            // Calculate hue
            if (maxVal == r)
            {
                hue = 60.0 * (std::fmod((g - b) / delta, 6.0));
            }
            else if (maxVal == g)
            {
                hue = 60.0 * (((b - r) / delta) + 2);
            }
            else if (maxVal == b)
            {
                hue = 60.0 * (((r - g) / delta) + 4);
            }
            if (hue < 0)
            {
                hue += 360.0;
            }
        }

        // Store valuyes in returnArray
        // Hue is a degree value between 0 and 360
        // Saturation is a float between 0 and 1
        // Luminance is a float between 0 and 1
        returnArray[i].hue = hue;
        returnArray[i].saturation = saturation;
        returnArray[i].lightness = luminance;

        return returnArray;
    }
}