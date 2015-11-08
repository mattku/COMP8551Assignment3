#include <Util.h>

#include <iostream>
#include <iomanip>
using namespace std;

void DisplayImage(Image& image, bool* display_channels)
{
	for(size_t i = 0; i < image.Height(); i++)
	{
		for(size_t j = 0; j < image.Width(); j++)
		{
			float* pix = image.At(i, j);
			cout<<"(";
			for(size_t ch = 0; ch < Image::NUM_CHANNELS; ch++)
			{
				if(display_channels[ch])
				{
					cout<<setprecision(2)<<pix[ch]<<" ";
				}
			}
			cout<<")   ";
		}
		cout<<endl;
	}
}

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

cl_mem CreateImageObject(Image& image_data, cl_context ctx, cl_mem_flags flags)
{
    cl_int err = 0;
    float* host_ptr = image_data.Serialize();
    // for(size_t i = 0; i < image_data.NumElements(); i++)
    // {
    // 	cout<<host_ptr[i]<<" "<<endl;
    // }
    //cout<<"serialize"<<endl;
    cl_image_format img_format;
    img_format.image_channel_order = CL_RGBA;
    img_format.image_channel_data_type = CL_FLOAT;
    cl_mem mem_object = clCreateImage2D(
        ctx, 
        flags, 
        &img_format, 
        image_data.Width(), 
        image_data.Height(), 
        image_data.RowPitch(), 
        host_ptr, 
        &err);
    if(err)
    {
        cerr<<"OpenCL Error: "<<ErrorString(err)<<endl;
        exit(1);
    }
    //delete[] host_ptr;
    return mem_object;
}
