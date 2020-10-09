#ifndef EulerianMagnification_H
#define	EulerianMagnification_H

#include <opencv2/core/core.hpp>

using namespace cv;

class EulerianMagnification 
{
	//Private Members
	Mat blurred;
	Mat lowpassHigh;
	Mat lowpassLow;
	Mat result;
	Size blurredSize;
	double fHigh;
	double fLow;
	int alpha;
public:
	//Public Members
	bool isFirstRun;
	Rect forehead;

	//Public Functions
	EulerianMagnification();
	void processFrame(Mat, Mat&);
};

#endif

