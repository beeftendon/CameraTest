#include <stdio.h>
#include <iostream>
#include <fstream>

#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;


// HiRes Windows timer, this part is usable only if compiled/run in Windows

#include <Windows.h>

double PCFreq = 0.0;
__int64 CounterStart = 0;

void StartCounter()
{
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li))
		cout << "QueryPerformanceFrequency failed!\n";

	PCFreq = double(li.QuadPart) / 1000.0;

	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
}
double GetCounter()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart - CounterStart) / PCFreq;
}

int main(int argc, char** argv)
{
	VideoCapture capture(CV_CAP_ANY);
	capture.set(CV_CAP_PROP_FPS, 240);
	int test = capture.get(CV_CAP_PROP_FPS);
	//capture.set(CV_CAP_PROP_EXPOSURE, -5);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, 200);
	capture.set(CV_CAP_PROP_FRAME_WIDTH, 200);
	
	if (!capture.isOpened())
	{
		fprintf(stderr, "ERROR: No camera detected\n");
		getchar();
		return -1;
	}

	namedWindow("Window", CV_WINDOW_AUTOSIZE);
	
	ofstream timefile;
	timefile.open("timedata.txt");

	while (1)
	{
		StartCounter();
		Mat frame;
		capture >> frame;
		//Mat frame = cvQueryFrame(capture);
		if (frame.empty())
		{
			fprintf(stderr, "ERROR: Frame is null\n");
			getchar();
			break;
		}
		imshow("Window", frame);
		char c = cvWaitKey(1);
		if (c == 27) break;

		timefile << GetCounter() << "\n";
		
	}

	//cvReleaseCapture(&capture);
	capture.release();
	destroyWindow("Window");
	timefile.close();
	return 0;
}