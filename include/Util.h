#pragma once

#include "Image.h"
#include <OpenCL/cl.h>

void DisplayImage(Image& image, bool* display_channels);
std::string ErrorString(cl_int);
cl_mem CreateImageObject(Image&, cl_context, cl_mem_flags);