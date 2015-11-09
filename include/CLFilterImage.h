#pragma once

#include "Image.h"
#include <OpenCL/cl.h>

class CLFilterImage
{
public:	
	CLFilterImage(cl_device_id, const char*, const char*);
	~CLFilterImage();

	void InitializeMemory(Image&, Image&, Image&);
	void Invoke();
	void ReadResult(Image&);
private:
	cl_device_id device_id_;
	cl_context context_;
	cl_program program_;
    cl_kernel kernel_;
    cl_command_queue queue_;
    cl_mem src_img_, dest_img_, filter_img_;
    size_t global_size_[2];

	void create_context();
	void build_program(const char*);
};