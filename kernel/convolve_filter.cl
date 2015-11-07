__kernel void convolve_filter(
    __write_only image2d_t dest_img, 
    __read_only image2d_t src_img, 
    __read_only image2d_t filter_image)
{
    const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST; 
    int2 coord = (int2)(get_global_id(1), get_global_id(0));
    int2 filter_dim = get_image_dim(filter_image);
    int2 filter_center = filter_dim / 2;
    float4 result = {0, 0, 0, 0};
	for(int f = 0; f < filter_dim.x; f++)
	{
		for(int g = 0; g < filter_dim.y; g++)
		{
			int2 filter_coord = {f, g};
			float4 src_pixel = read_imagef(src_img, smp, coord + filter_coord - filter_center);
			float4 filter_pixel = read_imagef(filter_image, smp, filter_coord);
			
			result += src_pixel * filter_pixel;
		}
	}
	write_imagef(dest_img, coord, result);
}
