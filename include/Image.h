#pragma once

#include <random>

class Image
{
public:	
	const static size_t NUM_CHANNELS;
	//make an image with all 0s.
	Image(size_t width, size_t height);
	//make an image with random values.
	Image(size_t width, size_t height, std::default_random_engine& rng);

	~Image();

	size_t Width() const;
	size_t Height() const;

	float* At(size_t row, size_t col);

	void* Serialize();
	void Deserialize(float* linear_data);

	size_t NumPixels() const;
	size_t NumElements() const;
	size_t RowElements() const;
	size_t RowPitch() const;
	size_t NumBytes() const;
private:
	size_t width_;
	size_t height_;
	float** pixel_data_;

};