#define PROGRAM_FILE "kernel/convolve_filter.cl"
#define KERNEL_FUNC "convolve_filter"

#define FILTER_HEIGHT 3
#define FILTER_WIDTH 3
#define IMAGE_HEIGHT 16
#define IMAGE_WIDTH 16

#include <Filter.h>
#include <Util.h>

#include <random>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <OpenCL/cl.h>

using namespace std;

string ErrorString(cl_int error_code) 
{
    switch (error_code) 
    {
    case CL_SUCCESS:                            
        return "Success!";
    case CL_DEVICE_NOT_FOUND:                   
        return "Device not found.";
    case CL_DEVICE_NOT_AVAILABLE:               
        return "Device not available";
    case CL_COMPILER_NOT_AVAILABLE:             
        return "Compiler not available";
    case CL_MEM_OBJECT_ALLOCATION_FAILURE:      
        return "Memory object allocation failure";
    case CL_OUT_OF_RESOURCES:                   
        return "Out of resources";
    case CL_OUT_OF_HOST_MEMORY:                 
        return "Out of host memory";
    case CL_PROFILING_INFO_NOT_AVAILABLE:       
        return "Profiling information not available";
    case CL_MEM_COPY_OVERLAP:                   
        return "Memory copy overlap";
    case CL_IMAGE_FORMAT_MISMATCH:              
        return "Image format mismatch";
    case CL_IMAGE_FORMAT_NOT_SUPPORTED:         
        return "Image format not supported";
    case CL_BUILD_PROGRAM_FAILURE:              
        return "Program build failure";
    case CL_MAP_FAILURE:                        
        return "Map failure";
    case CL_INVALID_VALUE:                      
        return "Invalid value";
    case CL_INVALID_DEVICE_TYPE:                
        return "Invalid device type";
    case CL_INVALID_PLATFORM:                   
        return "Invalid platform";
    case CL_INVALID_DEVICE:                     
        return "Invalid device";
    case CL_INVALID_CONTEXT:                    
        return "Invalid context";
    case CL_INVALID_QUEUE_PROPERTIES:           
        return "Invalid queue properties";
    case CL_INVALID_COMMAND_QUEUE:              
        return "Invalid command queue";
    case CL_INVALID_HOST_PTR:                   
        return "Invalid host pointer";
    case CL_INVALID_MEM_OBJECT:                 
        return "Invalid memory object";
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:    
        return "Invalid image format descriptor";
    case CL_INVALID_IMAGE_SIZE:                 
        return "Invalid image size";
    default:
        return "Unknown Error";
    }
}
/* Find a GPU or CPU associated with the first available platform */
cl_device_id create_device() {

    cl_platform_id platform;
    cl_device_id dev;
    int err;

/* Identify a platform */
    err = clGetPlatformIDs(1, &platform, NULL);
    if(err < 0) {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);
    } 

/* Access a device */
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
    if(err == CL_DEVICE_NOT_FOUND) {
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    }
    if(err < 0) {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);   
    }

    return dev;
}

/* Create program from a file and compile it */
cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename) {

    cl_program program;
    FILE *program_handle;
    char *program_buffer, *program_log;
    size_t program_size, log_size;
    int err;

/* Read program file and place content into buffer */
    program_handle = fopen(filename, "r");
    if(program_handle == NULL) {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);
    }
    fseek(program_handle, 0, SEEK_END);
    program_size = ftell(program_handle);
    rewind(program_handle);
    program_buffer = (char*)malloc(program_size + 1);
    program_buffer[program_size] = '\0';
    fread(program_buffer, sizeof(char), program_size, program_handle);
    fclose(program_handle);

/* Create program from file */
    program = clCreateProgramWithSource(ctx, 1, 
        (const char**)&program_buffer, &program_size, &err);
    if(err < 0) {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);
    }
    free(program_buffer);

/* Build program */
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(err < 0) {

/* Find size of log and print to std output */
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
            0, NULL, &log_size);
        program_log = (char*) malloc(log_size + 1);
        program_log[log_size] = '\0';
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
            log_size + 1, program_log, NULL);
        printf("%s\n", program_log);
        free(program_log);
        exit(1);
    }

    return program;
}

cl_context create_context(cl_device_id device)
{
    int err;
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if(err < 0) {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);
    }
    return context;
}


void create_image_object(cl_mem& mem_object, Image& image_data, const cl_context& ctx, const cl_mem_flags& flags)
{
    int err;
    void* host_ptr = image_data.Serialize();
    cl_image_format img_format;
    img_format.image_channel_order = CL_RGBA;
    img_format.image_channel_data_type = CL_FLOAT;
    cl_image_desc img_desc;
    img_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
    img_desc.image_width = image_data.Width();
    img_desc.image_height = image_data.Height();
    img_desc.image_row_pitch = image_data.RowPitch();
    img_desc.num_mip_levels = 0;
    img_desc.num_samples = 0;
    img_desc.buffer = NULL;
    mem_object = clCreateImage(ctx, flags, &img_format, &img_desc, host_ptr, &err);
    if(err)
    {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);
    }
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
    cl_context context;
    cl_program program;
    cl_kernel kernel;
    cl_command_queue queue;
    cl_int err;
    cl_mem src_img, dest_img, filter_img;

    device = create_device();
    context = create_context(device);

    /* Build program */
    program = build_program(context, device, PROGRAM_FILE);
    
    /* Create data buffers */
    create_image_object(src_img, src, context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR | CL_MEM_HOST_NO_ACCESS);
    create_image_object(dest_img, gpu_result, context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR | CL_MEM_HOST_READ_ONLY);
    create_image_object(filter_img, filter, context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR | CL_MEM_HOST_NO_ACCESS);

    /* Create a command queue */
    queue = clCreateCommandQueue(context, device, 0, &err);
    if(err < 0) {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);   
    };

    /* Create a kernel */
    kernel = clCreateKernel(program, KERNEL_FUNC, &err);
    if(err < 0) {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);
    }

    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dest_img);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &src_img);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &filter_img);
    if(err)
    {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);
    }

    size_t global_size[2] = {src.Width(), src.Height()};
    size_t local_size[2] = {1, 1};
    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, local_size, 0, NULL, NULL);
    if(err) 
    {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);
    }
    
    readResult(queue, dest_img, gpu_result);
    cout<<"Serial, CPU result: "<<endl;
    DisplayImage(dest, channels_mask);
    cout<<"GPU result: "<<endl;
    DisplayImage(gpu_result, channels_mask);
    /* Deallocate resources */
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);
    

    return 0;
}
