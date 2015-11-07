#pragma once

#include "Image.h"
#include "Sampler.h"

void MakeBlurFilter(Image& filter);
void ConvolveFilter(Image& dest, Image& src, Image& filter, Sampler& sampler);