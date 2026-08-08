#pragma once

#include "RGB.h"

class ColorController
{
  public:
    static RGB calculateRGB(float tint, float intensity);
};
