#include <Image.h>

#include <iostream>
#include <cstring>

using namespace std;
const size_t Image::NUM_CHANNELS = 4;

Image::Image(size_t width, size_t height)
	: width_(width), height_(height), pixel_data_(new float*[height])
{
	for(size_t i = 0; i < height_; i++)
	{
		pixel_data_[i] = new float[RowElements()];
		for(size_t j = 0; j < RowElements(); j++)
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
		pixel_data_[i] = new float[RowElements()];
		for(size_t j = 0; j < RowElements(); j++)
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

float* Image::Serialize()
{
	float* linear_data = new float[NumElements()];
	for(size_t i = 0; i < height_; i++)
	{
		size_t offset = i * RowElements();
		memcpy(linear_data + offset, pixel_data_[i], RowPitch());
	}
	return linear_data;
}

void Image::Deserialize(float* linear_data)
{
	for(size_t i = 0; i < height_; i++)
	{
		size_t offset = i * RowElements();
		memcpy(pixel_data_[i], linear_data + offset, RowPitch());
	}
}

size_t Image::NumPixels() const
{
	return width_ * height_;
}
size_t Image::NumElements() const
{
	return NumPixels() * NUM_CHANNELS;
}
size_t Image::RowPitch() const
{
	return RowElements() * sizeof(float);
}
size_t Image::NumBytes() const
{
	return NumElements() * sizeof(float);
}

size_t Image::RowElements() const
{
	return NUM_CHANNELS * width_;
}