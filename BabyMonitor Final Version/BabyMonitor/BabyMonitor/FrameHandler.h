#ifndef FRAMEHANDLER_H
#define	FRAMEHANDLER_H

#include <string>
#include <vector>
#include <opencv2/objdetect/objdetect.hpp>
#include "Face.h"
#include <highgui/highgui.hpp>

using namespace std;
using namespace cv;

class FrameHandler
{
	//Private Members
	double fps;
	CascadeClassifier classifier;
	Size minFaceSize;
	vector<Face> knownFaces;
	Mat previousFrame;
	bool isFirstRun;

	//Private Functions
	int getClosestFaceIndex(vector<Rect>::value_type);
	bool isAnyPulseValid();
public:
	//Public Members
	bool isMotionAvailable;

	//Public Functions
	FrameHandler(Size, double);
	void loadCascadeXml(string);
	void processFrame(Mat&);
	void detectFacesAndCalcHeartRate(Mat&);
	void detectMotion(Mat&, Mat&);
	void saveSnapshot(Mat);
};

#endif