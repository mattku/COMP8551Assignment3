#include <Image.h>

const size_t Image::NUM_CHANNELS = 3;

Image::Image(size_t width, size_t height)
	: width_(width), height_(height), pixel_data_(new float*[height])
{
	for(size_t i = 0; i < height_; i++)
	{
		pixel_data_[i] = new float[row_elements()];
		for(size_t j = 0; j < row_elements(); j++)
		{
			pixel_data_[i][j] = 0.0f;
		}
	}
}

Image::Image(size_t width, size_t height, std::default_random_engine& rng)
	: width_(width), height_(height), pixel_data_(new float*[height])
{
	for(size_t i = 0; i < height_; i++)
	{
		pixel_data_[i] = new float[row_elements()];
		for(size_t j = 0; j < row_elements(); j++)
		{
			pixel_data_[i][j] = (float)rng() / (float)rng.max();
		}
	}
}

Image::~Image()
{
	for(size_t i = 0; i < height_; i++)
	{
		delete[] pixel_data_[i];
	}
	delete[] pixel_data_;
}

size_t Image::Width() const
{
	return width_;
}

size_t Image::Height() const
{
	return height_;
}

float* Image::At(size_t row, size_t col)
{
	return pixel_data_[row] + (col * NUM_CHANNELS);
}

size_t Image::row_elements()
{
	return NUM_CHANNELS * width_;
}