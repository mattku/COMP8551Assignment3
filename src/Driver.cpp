#define PROGRAM_FILE "kernel/convolve_filter.cl"
#define KERNEL_FUNC "convolve_filter"

#define FILTER_HEIGHT 3
#define FILTER_WIDTH 3
#define IMAGE_HEIGHT 16
#define IMAGE_WIDTH 16

#include <Filter.h>
#include <Util.h>
#include <CLFilterImage.h>

#include <random>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <OpenCL/cl.h>

using namespace std;

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

void readResult(const cl_command_queue& queue, const cl_mem& mem, Image& image)
{
    size_t origin[3] = {0,0,0};
    size_t region[3] = {image.Width(), image.Height(), 1};
    size_t row_pitch = image.RowPitch();
    size_t slice_pitch = 0;
    float* data = new float[image.NumElements()];
    cl_int err = clEnqueueReadImage(queue, mem, CL_TRUE, origin, region, row_pitch, slice_pitch, data, 0, NULL, NULL);
    if(err)
    {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);
    }
    image.Deserialize(data);
}

int main(int argc, char** argv) 
{
    cout<<sizeof(CLFilterImage)<<endl;
    default_random_engine rng(0);
    Image src(IMAGE_WIDTH, IMAGE_HEIGHT, rng), filter(FILTER_WIDTH, FILTER_HEIGHT);
    Image dest(IMAGE_WIDTH, IMAGE_HEIGHT);
    Image gpu_result(IMAGE_WIDTH, IMAGE_HEIGHT);
    MakeBlurFilter(filter);
    bool channels_mask[4] = {true, false, false, false};
    Sampler sam;
    ConvolveFilter(dest, src, filter, sam);

    /* OpenCL structures */
    cl_device_id device;

    device = create_device(CL_DEVICE_TYPE_GPU);
    cout<<"create device"<<endl;
    CLFilterImage filter_gpu(device, PROGRAM_FILE, KERNEL_FUNC);
    cout<<"construct"<<endl;

    filter_gpu.InitializeMemory(src, gpu_result, filter);
    cout<<"init mem"<<endl;
    filter_gpu.Invoke();
    cout<<"invoke"<<endl;
    
    //readResult(queue, dest_img, gpu_result);
    cout<<"Serial, CPU result: "<<endl;
    DisplayImage(dest, channels_mask);
    cout<<"GPU result: "<<endl;
    DisplayImage(gpu_result, channels_mask);


    return 0;
}
