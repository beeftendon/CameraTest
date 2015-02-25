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
	CvCapture* capture = cvCaptureFromCAM(CV_CAP_ANY);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FPS, 240);

	if (!capture)
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
		Mat frame = cvQueryFrame(capture);
		if (frame.empty())
		{
			fprintf(stderr, "ERROR: Frame is null\n");
			getchar();
			break;
		}
		imshow("Window", frame);
		char c = cvWaitKey(10);
		if (c == 27) break;

		timefile << GetCounter() << "\n";
		
	}

	cvReleaseCapture(&capture);
	destroyWindow("Window");
	timefile.close();
	return 0;
}