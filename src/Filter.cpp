#include <Filter.h>

void MakeBlurFilter(Image& filter)
{
	size_t total = filter.Width() * filter.Height();
	float filter_value = 1.0f / (float) total;
	for(size_t i = 0; i < filter.Height(); i++)
	{
		for(size_t j = 0; j < filter.Width(); j++)
		{
			float* dest_pixel = filter.At(i, j);
			for(size_t ch = 0; ch < Image::NUM_CHANNELS; ch++)
			{
				dest_pixel[ch] = filter_value;
			}
		}
	}
}

void ConvolveFilter(Image& dest, Image& src, Image& filter, Sampler& sampler)
{
	int filter_center_y = filter.Height() / 2;
	int filter_center_x = filter.Width() / 2;
	//loop over each pixel (i, j)
	for(int i = 0; i < src.Height(); i++)
	{
		for(int j = 0; j < src.Width(); j++)
		{
			float* dest_pixel = sampler(dest, i, j);
			//loop over each pixel in the filter (f, g)
			for(int f = 0; f < filter.Height(); f++)
			{
				int y_offset = f - filter_center_y;
				for(int g = 0; g < filter.Width(); g++)
				{
					int x_offset = g - filter_center_x;
					float* src_pixel = sampler(src, i + y_offset, j + x_offset);
					float* filter_pixel = sampler(filter, f, g);
					//loop over each channel (ch)
					for(size_t ch = 0; ch < Image::NUM_CHANNELS; ch++)
					{
						dest_pixel[ch] += src_pixel[ch] * filter_pixel[ch];
					}
				}
			}
		}
	}
}