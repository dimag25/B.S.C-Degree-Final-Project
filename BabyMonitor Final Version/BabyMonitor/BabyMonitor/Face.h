#ifndef FACE_H
#define	FACE_H
#include <objdetect/objdetect.hpp>
#include "EulerianMagnification.h"

using namespace cv;
using namespace std;

class Face
{
	//Private members
	double heartRate;
	Mat1d rawSignal;
	Mat1d pulseSignal;
	Mat1d timestamps;
	EulerianMagnification em;

	//Private functions
	void updateSizeAndPosition(Rect&);
	void processFrame(Mat&, double);
	void drawOnFrame(Mat&);
	bool isSignalStable();
	void estimateHeartRate(double);
	void resetProcess();
public:
	//Public members
	Rect faceRect;
	bool isMatched;

	//Public functions
	Face(Rect&);
	int getClosestFaceIndex(vector<Rect>);
	bool doesValidHeartRateExist();
	void processFace(Mat&, double, Rect&);
	double getBPM();
};

#endif