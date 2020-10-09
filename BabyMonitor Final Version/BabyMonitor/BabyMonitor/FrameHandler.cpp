#include "FrameHandler.h"
#include <fstream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>

using namespace std;
using namespace cv;

//C-tor
FrameHandler::FrameHandler(Size faceSize, double _fps)
{
	minFaceSize = faceSize;
	fps = _fps;
	isFirstRun = true;
	isMotionAvailable = false;
	knownFaces.clear();
}

//Loads the LBP cascade for face detection
void FrameHandler::loadCascadeXml(string fileName)
{
	classifier.load(fileName);
}

//Processing frame
void FrameHandler::processFrame(Mat& frame)
{
	//Save frame before changes, in case motion detection is in order
	Mat savedFrame;
	frame.copyTo(savedFrame);
		
	detectFacesAndCalcHeartRate(frame);

	//If the pulse does not exist or is not valid, detect motion instead
	if (!isFirstRun && !isAnyPulseValid())
	{
		detectMotion(previousFrame, savedFrame);
		//Copy result back to original frame
		savedFrame.copyTo(frame);
	}

	savedFrame.copyTo(previousFrame);
	isFirstRun = false;
}

//Detects faces and handles them accordingly
void FrameHandler::detectFacesAndCalcHeartRate(Mat& frame)
{
	try
	{
		cvtColor(frame, frame, CV_BGR2RGB);

		//Detect face(s)
		vector<Rect> discoveredFaces;
		classifier.detectMultiScale(frame, discoveredFaces, 1.1, 3, 0, minFaceSize);

		int i;
		//If there are more previously detected faces than discovered faces
		if (knownFaces.size() > discoveredFaces.size()) //TODO: Consider clearing knownfaces
		{
			//Unmark all known faces
			for (i = 0; i < knownFaces.size(); i++)
				knownFaces[i].isMatched = false;

			//Match every discovered face to the closest known face
			for (i = 0; i < discoveredFaces.size(); i++)
			{
				int faceIndex = getClosestFaceIndex(discoveredFaces[i]);
				Face& face = knownFaces[faceIndex];
				face.isMatched = true;
				face.processFace(frame, fps, discoveredFaces[i]);
			}

			//Remove unmarked faces
			vector<Face> updatedNewFaces;
			for (i = 0; i < knownFaces.size(); i++)
			{
				if (knownFaces[i].isMatched)
					updatedNewFaces.push_back(knownFaces[i]);
			}
			knownFaces = updatedNewFaces;
		}
		else
		{
			//Match every discovered face to its' known face
			for (i = 0; i < knownFaces.size(); i++)
			{
				Face& knownFace = knownFaces[i];
				int faceIndex = knownFace.getClosestFaceIndex(discoveredFaces);
				knownFace.processFace(frame, fps, discoveredFaces[faceIndex]);
			}

			//Create new face objects for new faces
			for (; i < discoveredFaces.size(); i++)
			{
				Face newFace = Face(discoveredFaces[i]);
				knownFaces.push_back(newFace);
				newFace.processFace(frame, fps, discoveredFaces[i]);
			}
		}

		cvtColor(frame, frame, CV_RGB2BGR);
	}
	catch (out_of_range e)
	{
		cout << "Error while detecting & updating faces: " << e.what() << endl;
	}
}

//Returns the index of the known face that is closest to the discovered face
int FrameHandler::getClosestFaceIndex(vector<Rect>::value_type discoveredFace)
{
	int index = -1;
	int min = INT_MAX;

	//Search for the existing face's index with the minimum distance from the discovered face's top left corner
	for (int i = 0; i < knownFaces.size(); i++)
	{
		//Ignore marked faces (they have already been matched)
		if (knownFaces[i].isMatched) continue;

		//Calculate euclidean distance
		int distance = norm(discoveredFace.tl() - knownFaces[i].faceRect.tl());
		if (distance < min)
		{
			min = distance;
			index = i;
		}
	}

	return index;
}

//Determine if any of the known faces has a valid heart rate
bool FrameHandler::isAnyPulseValid()
{
	for (int i = 0; i < knownFaces.size(); i++)
	{
		if (knownFaces[i].doesValidHeartRateExist())
			return true;
	}
	return false;
}

//Detect motions in given frame (in comparison to previous frame)
void FrameHandler::detectMotion(Mat& oldFrame, Mat& currentFrame)
{
	try
	{
		Mat oldFrameGray, currFrameGray, frameDelta, frameThreshold;

		//Convert old frame to grayscale
		cvtColor(oldFrame, oldFrameGray, COLOR_BGR2GRAY);

		//Convert current frame to grayscale
		cvtColor(currentFrame, currFrameGray, COLOR_BGR2GRAY);

		//Calculate absolute difference between current frame and old frame (in grayscale form)
		absdiff(oldFrameGray, currFrameGray, frameDelta);

		//Calculate threshold from frame delta and blur it to reduce false positives and noises
		threshold(frameDelta, frameThreshold, 5, 255, CV_THRESH_BINARY);
		GaussianBlur(frameThreshold, frameThreshold, Size(3, 3), 0, 0);
		threshold(frameDelta, frameThreshold, 5, 255, CV_THRESH_BINARY);

		//getStructuringElement func- Returns a structuring element of the specified size and shape for morphological operations.
		//morphological operations is useful for finding the outline of an object that we want to detect in our video
		//Structural element used for finding the outline of the objects
		Mat morphologicalStructure = getStructuringElement(MORPH_RECT, Size(7, 7));

		// use function cv::erode for image filtering using morphological structure 
		//erode function helps us to mark only our movement objects and filter parts of frame that without movement
		//Mark moving objects and filter out the sections of the frame without movement
		erode(frameThreshold, frameThreshold, morphologicalStructure, Point(-1, -1));

		vector<vector<Point>> contours;
		String label = "no motion";

		//Find and draw contours of motion ,show label if no motion detects
		findContours(frameThreshold, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		if (contours.size() == 0)
		{
			putText(currentFrame, label, Point(30, 30), FONT_HERSHEY_PLAIN, 2.0, CV_RGB(255, 0, 0), 2.0);
			isMotionAvailable = false;
		}
		else isMotionAvailable = true;

		drawContours(currentFrame, contours, -1, Scalar(0, 255, 0), 1);
	}
	catch (exception& e)
	{
		cout << "Failed to detect motion: " << e.what() << endl;
	}
}

//Save frame snapshot to disk
void FrameHandler::saveSnapshot(Mat frame)
{
	try
	{
		//Snapshot
		stringstream sstr;
		sstr << "C:/BabyMonitor/snapshot.jpg";
		imwrite(sstr.str(), frame);
		
		for (int i = 0; i < knownFaces.size(); i++)
		{
			ofstream file;
			stringstream path;
			path << "C:/BabyMonitor/data_" << i << ".txt";
			file.open(path.str());
			//Json containing bpm & motion
			string json = "{\"bpm\":" + to_string(knownFaces[i].getBPM()) + ",\"motion\":" + to_string(isMotionAvailable) + "}";
			file << json;
			file.close();
		}
	}
	catch (exception e)
	{
		cout << "Failed to save snapshot: " << e.what() << endl;
	}
}