#pragma once

#define UNICODE
#define _UNICODE

#include <array>
#include "Monitor.h"
#include <iostream>

struct HSL
{
    double hue;
    double saturation;
    double lightness;
};

class Converter
{
private:
public:
    HSL RGBToHSL(std::array<std::array<WORD, 256>, 3> WordArray);
};