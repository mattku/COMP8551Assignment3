#include <CLFilterImage.h>
#include <iostream>
#include <Util.h>

using namespace std;

CLFilterImage::CLFilterImage(cl_device_id device_id, const char* program_file, const char* kernel_func)
	: device_id_(device_id){
	create_context();
	build_program(program_file);

	cl_int err;
	/* Create a command queue */
    queue_ = clCreateCommandQueue(context_, device_id_, 0, &err);
    if(err < 0) {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);   
    };

    /* Create a kernel */
    kernel_ = clCreateKernel(program_, kernel_func, &err);
    if(err < 0) {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);
    }
}

CLFilterImage::~CLFilterImage() {
	/* Deallocate resources */
    clReleaseKernel(kernel_);
    clReleaseCommandQueue(queue_);
	clReleaseProgram(program_);
    clReleaseContext(context_);
}

void CLFilterImage::InitializeMemory(Image& src, Image& dest, Image& filter) {
    src_img_ = CreateImageObject(src, context_, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR | CL_MEM_HOST_NO_ACCESS);

    dest_img_ = CreateImageObject(dest, context_, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR | CL_MEM_HOST_READ_ONLY);
    filter_img_ = CreateImageObject(filter, context_, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR | CL_MEM_HOST_NO_ACCESS);
    global_size_[0] = src.Width();
    global_size_[1] = src.Height();
}

void CLFilterImage::Invoke() {
	cl_int err;

	err = clSetKernelArg(kernel_, 0, sizeof(cl_mem), &dest_img_);
    err |= clSetKernelArg(kernel_, 1, sizeof(cl_mem), &src_img_);
    err |= clSetKernelArg(kernel_, 2, sizeof(cl_mem), &filter_img_);
    if(err)
    {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);
    }

	size_t local_size_[2] = {1, 1};
    err = clEnqueueNDRangeKernel(queue_, kernel_, 2, NULL, global_size_, local_size_, 0, NULL, NULL);
    if(err) 
    {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);
    }
}

void CLFilterImage::ReadResult(Image& dest) {
	size_t origin[3] = {0,0,0};
    size_t region[3] = {dest.Width(), dest.Height(), 1};
    size_t row_pitch = dest.RowPitch();
    size_t slice_pitch = 0;
    float* data = new float[dest.NumElements()];
    cl_int err = clEnqueueReadImage(queue_, dest_img_, CL_TRUE, origin, region, row_pitch, slice_pitch, data, 0, NULL, NULL);
    if(err)
    {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);
    }
    dest.Deserialize(data);
    delete[] data;
}

void CLFilterImage::create_context()
{
    int err;
    context_ = clCreateContext(NULL, 1, &device_id_, NULL, NULL, &err);
    if(err < 0) {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);
    }
}

/* Create program from a file and compile it */
void CLFilterImage::build_program(const char* filename) {
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
	program_ = clCreateProgramWithSource(context_, 1, 
		(const char**)&program_buffer, &program_size, &err);
	if(err < 0) {
		cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
		exit(1);
	}
	free(program_buffer);

	/* Build program */
	err = clBuildProgram(program_, 0, NULL, NULL, NULL, NULL);
	if(err < 0) {

		/* Find size of log and print to std output */
		clGetProgramBuildInfo(program_, device_id_, CL_PROGRAM_BUILD_LOG, 
			0, NULL, &log_size);
		program_log = (char*) malloc(log_size + 1);
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program_, device_id_, CL_PROGRAM_BUILD_LOG, 
			log_size + 1, program_log, NULL);
		printf("%s\n", program_log);
		free(program_log);
		exit(1);
	}
}
