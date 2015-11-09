#define PROGRAM_FILE "kernel/convolve_filter.cl"
#define KERNEL_FUNC "convolve_filter"

#define FILTER_HEIGHT 15
#define FILTER_WIDTH 15
#define IMAGE_HEIGHT 4096
#define IMAGE_WIDTH 4096

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

#include <OpenCL/cl.h>

using namespace std;
using namespace std::chrono;

/* Find a GPU or CPU associated with the first available platform */
cl_device_id create_device(cl_device_type device_type) {

    cl_platform_id platforms[3];
    cl_uint num_platforms;
    cl_device_id dev;
    int err;
    bool device_available = false;

    /* Identify a platform */
    err = clGetPlatformIDs(3, platforms, &num_platforms);
    if(err < 0) {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);
    } 

    /* Access a device */
    for(size_t i = 0; i < num_platforms; i++) {
        err = clGetDeviceIDs(platforms[i], device_type, 1, &dev, NULL);
        if(err && err != CL_DEVICE_NOT_FOUND) {
            cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
            exit(1);   
        }
        else if(!err) {
            device_available = true;
        }
    }   
    if(!device_available) {
        cerr<<"No devices Error: "<<endl;
        exit(1);
    }
    return dev;
}

void DisplayResult(Image& result, string msg, duration<double> time_taken)
{
    bool channels_mask[4] = {true, false, false, false};
    cout<<msg<<endl;

    //*****UNCOMMENT THIS TO DISPLAY THE R CHANNEL OF THE RESULTING IMAGE*****
    //DisplayImage(result, channels_mask);
    cout<<"Time: "<<time_taken.count()<<endl;
}

void DisplayCombinedResult(Image& im1, Image& im2, string msg, duration<double> time_taken)
{
    bool channels_mask[4] = {true, false, false, false};
    cout<<msg<<endl;

    //*****UNCOMMENT THESE TO DISPLAY THE R CHANNEL OF THE RESULTING IMAGES*****
    //DisplayImage(im_1, channels_mask);
    //DisplayImage(im_2, channels_mask);
    cout<<"Time: "<<time_taken.count()<<endl;
}

void RunSerial(Image& src, Image& filter)
{
    Image result(IMAGE_WIDTH, IMAGE_HEIGHT);

    Timer t;
    t.Start();
    //////////////////////////////////////////
    Sampler sam;
    ConvolveFilter(result, src, filter, sam);
    //////////////////////////////////////////
    t.End();
    DisplayResult(result, "Serial, CPU result: ", t.ElapsedTime());
}

void RunGPU(Image& src, Image& filter)
{
    Image result(IMAGE_WIDTH, IMAGE_HEIGHT);
    /* OpenCL structures */
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
    DisplayResult(result, "GPU result: ", t.ElapsedTime());
}

void RunCPU(Image& src, Image& filter)
{
    Image result(IMAGE_WIDTH, IMAGE_HEIGHT);
    /* OpenCL structures */
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
    DisplayResult(result, "CPU result: ", t.ElapsedTime());
}

void RunSplit(Image& src, Image& filter)
{
    Image result_gpu(IMAGE_WIDTH, IMAGE_HEIGHT);
    Image result_cpu(IMAGE_WIDTH, IMAGE_HEIGHT);
    /* OpenCL structures */
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
    DisplayCombinedResult(result_gpu, result_cpu, "Combined result: ", t.ElapsedTime());

}

int main(int argc, char** argv) 
{
    default_random_engine rng(0);
    Image src(IMAGE_WIDTH, IMAGE_HEIGHT, rng), filter(FILTER_WIDTH, FILTER_HEIGHT);
    MakeBlurFilter(filter);

    RunGPU(src, filter);

    RunCPU(src, filter);

    RunSplit(src, filter);

    RunSerial(src, filter);

    return 0;
}
