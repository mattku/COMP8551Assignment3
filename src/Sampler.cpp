#include <Sampler.h>

//get the nearest point in the image.
float* Sampler::operator()(Image& im, int row, int col)
{
	//clamp the row.
	row = clamp(row, 0, (int)im.Height());
	col = clamp(col, 0, (int)im.Width());
	return im.At(row, col);
}