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