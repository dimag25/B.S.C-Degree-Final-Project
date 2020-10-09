#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <imgproc/types_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <ctime>
#include "FrameHandler.h"

using namespace cv;
using namespace std;

void main()
{
	VideoCapture capture(0);
	
	//Calculate min face size
	int width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
	int height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	Size minFaceSize = Size(min(width, height)*0.4, min(width, height) * 0.4);

	//Init frame handler
	double fps = capture.get(CV_CAP_PROP_FPS);
	FrameHandler frame_handler = FrameHandler(minFaceSize, fps);
	frame_handler.loadCascadeXml("lbpcascade_frontalface.xml");

	Mat frame;
	clock_t snapshotTime = clock();
	while (true)
	{
		try
		{
			//Get frame (for heart rate calculation & motion detection)
			capture >> frame;

			//If ESC is pressed, stop recording if there is no frame, stop recording
			if (waitKey(1) == 27 || frame.empty())
				break;

			//Process frame
			frame_handler.processFrame(frame);

			//Save snapshot to disk every 10 seconds
			if (double(clock() - snapshotTime) / CLOCKS_PER_SEC >= 10)
			{
				snapshotTime = clock();
				frame_handler.saveSnapshot(frame);
			}

			//Display result on window
			imshow("Baby Monitor", frame);
		}
		catch (exception e)
		{
			cout << "Exception in main loop: " << e.what() << endl;
		}
	}

	capture.release();
	destroyAllWindows();
}