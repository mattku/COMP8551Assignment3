#include <Sampler.h>
#include <iostream>

//get the nearest point in the image.
float* Sampler::operator()(Image& im, int row, int col)
{
	//clamp the row.
	row = clamp(row, 0, (int)im.Height() - 1);
    //clamp the col.
	col = clamp(col, 0, (int)im.Width() - 1);
	return im.At(row, col);
}