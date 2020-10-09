#include "Face.h"
#include "OpenCVExtension.h"
#include <iostream>

//C-tor
Face::Face(Rect& rect) : heartRate(0), faceRect(rect), isMatched(false)
{}

//Returns the index of discovered face that is closest to this face
int Face::getClosestFaceIndex(vector<Rect> faces)
{
	int index = -1;
	int min = INT_MAX;
	for (int i = 0; i < faces.size(); i++)
	{
		//Calculate euclidean distance
		int distance = norm(faceRect.tl() - faces[i].tl());
		if (distance < min)
		{
			min = distance;
			index = i;
		}
	}
	return index;
}

bool Face::doesValidHeartRateExist()
{
	return heartRate >= 30;
}

//Calls functions that process frame
void Face::processFace(Mat& frame, double fps, Rect& prevFaceRect)
{
	updateSizeAndPosition(prevFaceRect);
	processFrame(frame, fps);
	drawOnFrame(frame);
}

//Update face rectangle and forehead rectangle location
void Face::updateSizeAndPosition(Rect& prevFaceRect)
{
	//Update face rectangle position and size
	Point p = faceRect.tl() - prevFaceRect.tl();
	double d = sqrt(p.x * p.x + p.y * p.y);
	double ratio = 3 * d / faceRect.width;
	interpolate(faceRect, prevFaceRect, faceRect, min(1.0, ratio));

	//Update forehead rectangle relatively to face
	Point foreheadTL = faceRect.tl() + Point(faceRect.size().width * 0.3, 3);
	Size foreheadSize = Size(faceRect.width * 0.4, faceRect.height * 0.25);
	em.forehead = Rect(foreheadTL, foreheadSize);
}

//Processing frame
void Face::processFrame(Mat& frame, double fps)
{
	try
	{
		//Apply Eulerian Video Magnification on region of interest (forehead)
		Mat roi = frame(em.forehead);
		em.processFrame(roi, roi);

		//Once exceeding 100 raws, remove the first raw in each iteration
		if (rawSignal.rows >= 100)
		{
			int total = rawSignal.rows;
			rawSignal.rowRange(1, total).copyTo(rawSignal.rowRange(0, total - 1));
			rawSignal.pop_back();
			timestamps.rowRange(1, total).copyTo(timestamps.rowRange(0, total - 1));
			timestamps.pop_back();
		}

		//Extract raw signal value
		double greenChnl = mean(roi)(1); //Extract the green channel
		rawSignal.push_back<double>(greenChnl);
		timestamps.push_back<double>(getTickCount());

		if (isSignalStable())
		{
			if (fps == 0) //Calculate FPS
			{
				double diff = (timestamps(timestamps.rows - 1) - timestamps(0)) * 1000. / getTickFrequency();
				fps = timestamps.rows * 1000 / diff;
			}

			//Process raw signal
			detrend<double>(rawSignal, pulseSignal, fps / 2);
			normalization(pulseSignal, pulseSignal);
			meanFilter(pulseSignal, pulseSignal);

			estimateHeartRate(fps);
		}
		else
		{
			cout << "signal unstable" << endl;
			resetProcess();
		}
	}
	catch (exception e)
	{
		cout << "FACE: Failed to process frame: " << e.what() << endl;
	}
}

//Determine if signal is stable enough to detect pulse
bool Face::isSignalStable()
{
	Scalar rawStdDev;
	meanStdDev(rawSignal, Scalar(), rawStdDev);
	return rawStdDev(0) <= 200 * 0.5;
}

//Calculate heart rate
void Face::estimateHeartRate(double fps)
{
	try
	{
		Mat1d powerSpectrum;

		//Apply Discrete Fourier Transform
		dft(pulseSignal, powerSpectrum);

		int total = rawSignal.rows;

		//Band limit
		int low = total * 40 / 60 / fps + 1;
		int high = total * 240 / 60 / fps + 1;
		powerSpectrum.rowRange(0, min(static_cast<int>(low), static_cast<int>(total))) = ZERO;
		powerSpectrum.pop_back(max(min(static_cast<int>(total - high), static_cast<int>(total)), 0));

		//Power spectrum
		pow(powerSpectrum, 2, powerSpectrum);

		if (!powerSpectrum.empty())
		{
			//Get max index of max power spectrum
			int idx[2];
			minMaxIdx(powerSpectrum, NULL, NULL, NULL, &idx[0]);

			//Calculate heart rate
			heartRate = idx[0] * fps * 30 / total;

			if (heartRate < 30 || heartRate != heartRate)
				heartRate = 0;

			cout << "BPM: " << heartRate << endl;
		}
	}
	catch (exception e)
	{
		cout << "Failed to calculate heart rate: " << e.what() << endl;
	}
}

//Reset all the components
void Face::resetProcess()
{
	//Reset Eulerian Magnidication
	em.isFirstRun = true;

	//Clears raw signal
	rawSignal.pop_back(rawSignal.rows);
	timestamps.pop_back(timestamps.rows);

	//Reset pulse signal
	if (pulseSignal.rows == rawSignal.rows)
		pulseSignal = 0;
	else pulseSignal = Mat1d::zeros(rawSignal.rows, 1);

	//Reset heart rate
	heartRate = 0;
}

void Face::drawOnFrame(Mat& frame)
{
	try
	{
		rectangle(frame, faceRect, BLUE);
		rectangle(frame, em.forehead, GREEN);

		//Pulse line
		Point bl = faceRect.tl() + Point(0, faceRect.height);
		Point g;
		for (int i = 0; i < rawSignal.rows; i++)
		{
			g = bl + Point(i, -pulseSignal(i) * 10 - 50);
			line(frame, g, g, RED, 2);
		}

		//Heart rate
		stringstream ss;
		ss.precision(3);
		ss << heartRate;
		putText(frame, ss.str(), bl, FONT_HERSHEY_PLAIN, 2, RED, 2);
	}
	catch (exception e)
	{
		cout << "Failed to draw on frame: " << e.what() << endl;
	}
}

double Face::getBPM()
{
	return heartRate;
}