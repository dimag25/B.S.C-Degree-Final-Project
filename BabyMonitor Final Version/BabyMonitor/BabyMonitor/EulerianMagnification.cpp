#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/core/core.hpp>
#include <iostream>
#include "EulerianMagnification.h"

using namespace cv;
using namespace std;

//C-tor
EulerianMagnification::EulerianMagnification()
{
	isFirstRun = true;
	blurredSize = Size(10, 10);
	fLow = 70 / 60. / 10;
	fHigh = 80 / 60. / 10;
	alpha = 200;
}

//Processing frame
void EulerianMagnification::processFrame(Mat input, Mat& output)
{
	try
	{
		//Convert source to float
		Mat srcFloat;
		input.convertTo(srcFloat, CV_32F);

		//Apply spatial filter: blur and downsample
		resize(srcFloat, blurred, blurredSize, 0, 0, CV_INTER_AREA);

		if (isFirstRun)
		{
			isFirstRun = false;
			blurred.copyTo(lowpassHigh);
			blurred.copyTo(lowpassLow);
			input.copyTo(output);
		}
		else
		{
			//Apply temporal filter: subtraction of two lowpass filters
			lowpassHigh = lowpassHigh * (1 - fHigh) + fHigh * blurred;
			lowpassLow = lowpassLow * (1 - fLow) + fLow * blurred;
			blurred = lowpassHigh - lowpassLow;

			//Amplification
			blurred *= alpha;

			//Resize back to original size
			resize(blurred, result, input.size(), 0, 0, CV_INTER_LINEAR);

			//Add back to original frame
			result += srcFloat;

			//Convert to 8 bit
			result.convertTo(output, CV_8U);
		}
	}
	catch (exception e)
	{
		cout << "EM: Failed to process frame: " << e.what() << endl;
	}
}