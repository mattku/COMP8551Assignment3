#pragma once

#include "Image.h"

struct Sampler
{
public:
	float* operator()(Image& im, int row, int col);
};

template <typename T>
T clamp(const T& n, const T& lower, const T& upper) {
  return std::max(lower, std::min(n, upper));
}
