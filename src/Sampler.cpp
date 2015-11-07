#include <Sampler.h>
#include <iostream>

//get the nearest point in the image.
float* Sampler::operator()(Image& im, int row, int col)
{
	//clamp the row.
	row = clamp(row, 0, (int)im.Height() - 1);
	col = clamp(col, 0, (int)im.Width() - 1);
	//std::cout<<"r:"<<row<<" c:"<<col<<std::endl;
	return im.At(row, col);
}