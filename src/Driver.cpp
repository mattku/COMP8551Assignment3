#define PROGRAM_FILE "kernel/add_numbers.cl"
#define KERNEL_FUNC "add_numbers"
#define DATA_SIZE 1023lu
#define LOCAL_SIZE 64

#define IMAGE_WIDTH 16
#define IMAGE_HEIGHT 16

#define FILTER_WIDTH 3
#define FILTER_HEIGHT 3

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

size_t getGlobalSize(size_t array_size)
{
    return array_size / 8;
}

size_t getNumWorkGroups(size_t array_size)
{
    return getGlobalSize(array_size) / LOCAL_SIZE;
}

/* Find a GPU or CPU associated with the first available platform */
cl_device_id create_device() {

   cl_platform_id platform;
   cl_device_id dev;
   int err;

   /* Identify a platform */
   err = clGetPlatformIDs(1, &platform, NULL);
   if(err < 0) {
      perror("Couldn't identify a platform");
      exit(1);
   } 

   /* Access a device */
   err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
   if(err == CL_DEVICE_NOT_FOUND) {
      err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
   }
   if(err < 0) {
      perror("Couldn't access any devices");
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
      perror("Couldn't find the program file");
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
      perror("Couldn't create the program");
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
        perror("Couldn't create a context");
        exit(1);
    }
    return context;
}

void init_data(float* data, size_t data_size, size_t array_size, float* sumData)
{
    /* Initialize data */
    for(size_t i=0; i<data_size; i++) {
        data[i] = 1.0f*i;
    }
    for(size_t i = data_size; i < array_size; i++)
    {
        data[i] = 0.0f;
    }
    for(int i = 0; i < getNumWorkGroups(array_size); i++) {
        sumData[i] = 0;
    }
}

void uploadData(cl_context context, float* data, size_t array_size, float* sumData, cl_mem& input_buffer, cl_mem& output_buffer)
{
    int err;
    input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY |
                                  CL_MEM_COPY_HOST_PTR, array_size * sizeof(float), data, &err);
    output_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE |
                                CL_MEM_COPY_HOST_PTR, getNumWorkGroups(array_size) * sizeof(float), sumData, &err);
    if(err < 0) {
        perror("Couldn't create a buffer");
        exit(1);
    };
}

//round up to a multiple of local_size.
size_t getPaddedArraySize(size_t data_size, size_t local_size)
{
    size_t rem = data_size % local_size;
    if(rem != 0)
    {
        return data_size - rem + local_size;
    }
    else
    {
        return data_size;
    }
}

void checkResults(float* local_sums, size_t array_size)
{
    float actual_sum = 0.0f;
    for(int i=0; i<getNumWorkGroups(array_size); i++) {
        actual_sum += local_sums[i];
    }
    float expected_sum = 1.0f * DATA_SIZE/2*(DATA_SIZE-1);
    printf("Computed sum = %.1f.\n", actual_sum);
    float sum_error = fabs(expected_sum - actual_sum);
    if(sum_error > 0.5)
        printf("Check failed. Expected %f, was %f (err = %f)\n", expected_sum, actual_sum, sum_error);
    else
        printf("Check passed.\n");
}


int main(int argc, char** argv) 
{
    default_random_engine rng(0);
    Image src(IMAGE_WIDTH, IMAGE_HEIGHT, rng), filter(FILTER_WIDTH, FILTER_HEIGHT);
    Image dest(IMAGE_WIDTH, IMAGE_HEIGHT);
    MakeBlurFilter(filter);
    bool channels_mask[3] = {true, false, false};
    cout<<"BEFORE"<<endl;
    DisplayImage(src, channels_mask);
    Sampler sam;
    ConvolveFilter(dest, src, filter, sam);
    cout<<"AFTER"<<endl;
    DisplayImage(dest, channels_mask);

   /* OpenCL structures */
   cl_device_id device;
   cl_context context;
   cl_program program;
   cl_kernel kernel;
   cl_command_queue queue;
   cl_int err;
   size_t array_size = getPaddedArraySize(DATA_SIZE, LOCAL_SIZE);
   /* Data and buffers */
   float* data = new float[array_size];
   float* sum = new float[getNumWorkGroups(array_size)];
   cl_mem input_buffer, sum_buffer;

   init_data(data, DATA_SIZE, array_size, sum);
   printf("data_size: %lu array_size: %lu workGroups: %lu\n", DATA_SIZE, array_size, getNumWorkGroups(array_size));
   /* Create device and context */
   device = create_device();
    context = create_context(device);

   /* Build program */
   program = build_program(context, device, PROGRAM_FILE);
    
    /* Create data buffers */
   uploadData(context, data, array_size, sum, input_buffer, sum_buffer);
   

   /* Create a command queue */
   queue = clCreateCommandQueue(context, device, 0, &err);
   if(err < 0) {
      perror("Couldn't create a command queue");
      exit(1);   
   };

   /* Create a kernel */
   kernel = clCreateKernel(program, KERNEL_FUNC, &err);
   if(err < 0) {
      perror("Couldn't create a kernel");
      exit(1);
   };

   /* Create kernel arguments */
   err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer);
   err |= clSetKernelArg(kernel, 1, LOCAL_SIZE * sizeof(float), NULL);
   err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &sum_buffer);
   if(err < 0) {
      perror("Couldn't create a kernel argument");
      exit(1);
   }

    size_t global_size = getGlobalSize(array_size);
    size_t local_size = LOCAL_SIZE;
   /* Enqueue kernel */
   err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, 
         &local_size, 0, NULL, NULL); 
   if(err < 0) {
      perror("Couldn't enqueue the kernel");
      exit(1);
   }

   /* Read the kernel's output */
   err = clEnqueueReadBuffer(queue, sum_buffer, CL_TRUE, 0, 
         getNumWorkGroups(array_size) * sizeof(float), sum, 0, NULL, NULL);
   if(err < 0) {
      perror("Couldn't read the buffer");
      exit(1);
   }

   checkResults(sum, array_size);
   /* Check result */

   /* Deallocate resources */
   clReleaseKernel(kernel);
   clReleaseMemObject(sum_buffer);
   clReleaseMemObject(input_buffer);
   clReleaseCommandQueue(queue);
   clReleaseProgram(program);
   clReleaseContext(context);
    
   delete[] data;
   
   return 0;
}
