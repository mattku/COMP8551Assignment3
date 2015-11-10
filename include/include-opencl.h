#pragma once

#if defined(__APPLE__) && defined(__MACH__)
    #include <OpenCL/cl.h>
#elif defined(__linux__)
    #include <CL/cl.h>
#endif