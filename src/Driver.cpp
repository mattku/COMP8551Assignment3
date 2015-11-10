#define PROGRAM_FILE "kernel/convolve_filter.cl"
#define KERNEL_FUNC "convolve_filter"

#define FILTER_HEIGHT 15
#define FILTER_WIDTH 15
#define IMAGE_HEIGHT 4096
#define IMAGE_WIDTH 4096

#define FILTER_HEIGHT_SMALL 3
#define FILTER_WIDTH_SMALL 3
#define IMAGE_HEIGHT_SMALL 16
#define IMAGE_WIDTH_SMALL 16

#include <Filter.h>
#include <Util.h>
#include <CLFilterImage.h>
#include <Timer.h>

#include <random>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "include-opencl.h"

using namespace std;
using namespace std::chrono;

/* Find a device associated with any platform */
cl_device_id create_device(cl_device_type device_type) {

    cl_platform_id platforms[3];
    cl_uint num_platforms;
    cl_device_id dev;
    cl_int err = 0;
    bool device_available = false;

    /* Identify a platform */
    err = clGetPlatformIDs(3, platforms, &num_platforms);
    if(err) {
        cerr<<"OpenCL Error clGetPlatformIDs: "<<ErrorString(err)<<" "<<err<<endl;
        exit(1);
    } 

    /* Access a device */
    for(size_t i = 0; i < num_platforms; i++) {
        err = clGetDeviceIDs(platforms[i], device_type, 1, &dev, NULL);
        if(err && err != CL_DEVICE_NOT_FOUND) {
            cerr<<"OpenCL Error clGetDeviceIDs: "<<ErrorString(err)<<endl;
            exit(1);   
        }
        else if(!err) {
            device_available = true;
        }
    }   
    if(!device_available) {
        cerr<<"No devices available"<<endl;
        exit(1);
    }
    return dev;
}

void DisplayResult(string msg, duration<double> time_taken)
{
    cout<<msg<<endl
        <<"Time: "<<time_taken.count()<<" seconds "<<endl;
}

duration<double> RunSerial(Image& result, Image& src, Image& filter)
{
    Sampler sam;
    Timer t;
    t.Start();
    //////////////////////////////////////////
    ConvolveFilter(result, src, filter, sam);
    //////////////////////////////////////////
    t.End();
    return t.ElapsedTime();
}

duration<double> RunGPU(Image& result, Image& src, Image& filter)
{
    cl_device_id device;

    device = create_device(CL_DEVICE_TYPE_GPU);
    CLFilterImage filterer(device, PROGRAM_FILE, KERNEL_FUNC);
    Timer t;
    t.Start();
    /////////////////////////////////////////////////
    filterer.InitializeMemory(src, result, filter);
    filterer.Invoke();
    filterer.ReadResult(result);
    /////////////////////////////////////////////////
    t.End();
    return t.ElapsedTime();
}

duration<double> RunCPU(Image& result, Image& src, Image& filter)
{
    cl_device_id device;

    device = create_device(CL_DEVICE_TYPE_CPU);
    CLFilterImage filterer(device, PROGRAM_FILE, KERNEL_FUNC);
    Timer t;
    t.Start();
    /////////////////////////////////////////////////
    filterer.InitializeMemory(src, result, filter);
    filterer.Invoke();
    filterer.ReadResult(result);
    /////////////////////////////////////////////////
    t.End();
    return t.ElapsedTime();
}

duration<double> RunSplit(Image& result_gpu, Image& result_cpu, Image& src, Image& filter)
{
    cl_device_id gpu_device, cpu_device;

    gpu_device = create_device(CL_DEVICE_TYPE_GPU);
    cpu_device = create_device(CL_DEVICE_TYPE_CPU);
    CLFilterImage gpu_filter(gpu_device, PROGRAM_FILE, KERNEL_FUNC);
    CLFilterImage cpu_filter(cpu_device, PROGRAM_FILE, KERNEL_FUNC);
    size_t offset_gpu[2] = {0, 0};
    size_t offset_cpu[2] = {0, IMAGE_HEIGHT/2};
    size_t global_size[2] = {IMAGE_WIDTH, IMAGE_HEIGHT/2};
    Timer t;
    t.Start();
    /////////////////////////////////////////////////
    gpu_filter.InitializeMemory(src, result_gpu, filter);
    cpu_filter.InitializeMemory(src, result_cpu, filter);
    gpu_filter.InvokeSubImage(offset_gpu, global_size);
    cpu_filter.InvokeSubImage(offset_cpu, global_size);
    gpu_filter.ReadResult(result_gpu);
    cpu_filter.ReadResult(result_cpu);
    /////////////////////////////////////////////////
    t.End();
    return t.ElapsedTime();
}

void ShowThatItWorks(default_random_engine& rng)
{
    //initialize the source image randomly in (0,1)
    Image src(IMAGE_WIDTH_SMALL, IMAGE_HEIGHT_SMALL, rng), 
    //others are initialized to 0
          filter(FILTER_WIDTH_SMALL, FILTER_HEIGHT_SMALL),
          dest_serial(IMAGE_WIDTH_SMALL, IMAGE_HEIGHT_SMALL),
          dest_gpu(IMAGE_WIDTH_SMALL, IMAGE_HEIGHT_SMALL),
          dest_cpu(IMAGE_WIDTH_SMALL, IMAGE_HEIGHT_SMALL),
          dest_combined_gpu(IMAGE_WIDTH_SMALL, IMAGE_HEIGHT_SMALL),
          dest_combined_cpu(IMAGE_WIDTH_SMALL, IMAGE_HEIGHT_SMALL);
    //initialize the filter
    MakeBlurFilter(filter);

    bool channels_mask[4] = {true, false, false, false}; //display only the R channel, for readability.


    DisplayResult("GPU: ", RunGPU(dest_gpu, src, filter));
    DisplayImage(dest_gpu, channels_mask);

    DisplayResult("CPU: ", RunCPU(dest_cpu, src, filter));
    DisplayImage(dest_cpu, channels_mask);

    DisplayResult("GPU/CPU Combined: ", RunSplit(dest_combined_gpu, dest_combined_cpu, src, filter));
    DisplayImage(dest_combined_gpu, channels_mask);
    DisplayImage(dest_combined_cpu, channels_mask);

    DisplayResult("Serial, CPU: ", RunSerial(dest_serial, src, filter));
    DisplayImage(dest_serial, channels_mask);
}

void DoItForReal(default_random_engine& rng)
{
    //initialize the source image randomly in (0,1)
    Image src(IMAGE_WIDTH, IMAGE_HEIGHT, rng), 
    //others are initialized to 0
          filter(FILTER_WIDTH, FILTER_HEIGHT),
          dest_serial(IMAGE_WIDTH, IMAGE_HEIGHT),
          dest_gpu(IMAGE_WIDTH, IMAGE_HEIGHT),
          dest_cpu(IMAGE_WIDTH, IMAGE_HEIGHT),
          dest_combined_gpu(IMAGE_WIDTH, IMAGE_HEIGHT),
          dest_combined_cpu(IMAGE_WIDTH, IMAGE_HEIGHT);

    MakeBlurFilter(filter);

    DisplayResult("GPU: ", RunGPU(dest_gpu, src, filter));

    DisplayResult("CPU: ", RunCPU(dest_cpu, src, filter));

    DisplayResult("GPU/CPU Combined: ", RunSplit(dest_combined_gpu, dest_combined_cpu, src, filter));

    DisplayResult("Serial, CPU: ", RunSerial(dest_serial, src, filter));
}

int main(int argc, char** argv) 
{
    default_random_engine rng(0);
    cout<<"Running on small images to demonstrate correctness..."<<endl;
    ShowThatItWorks(rng);
    cout<<"Running on large images, this may take a while..."<<endl;
    DoItForReal(rng);
    return 0;
}
