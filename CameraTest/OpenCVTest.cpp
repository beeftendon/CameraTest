//#include <stdio.h>
//#include <iostream>
//#include <fstream>
//
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/features2d/features2d.hpp>
//
//using namespace std;
//using namespace cv;
//
//
//// HiRes Windows timer, this part is usable only if compiled/run in Windows
//
//#include <Windows.h>
//
//double PCFreq = 0.0;
//__int64 CounterStart = 0;
//
//void StartCounter()
//{
//	LARGE_INTEGER li;
//	if (!QueryPerformanceFrequency(&li))
//		cout << "QueryPerformanceFrequency failed!\n";
//
//	PCFreq = double(li.QuadPart) / 1000.0;
//
//	QueryPerformanceCounter(&li);
//	CounterStart = li.QuadPart;
//}
//double GetCounter()
//{
//	LARGE_INTEGER li;
//	QueryPerformanceCounter(&li);
//	return double(li.QuadPart - CounterStart) / PCFreq;
//}
//
//int main(int argc, char** argv)
//{
//	int lowThreshold = 60;
//	int thresholdRatio = 10;
//	int kernelSize = 3;
//
//	vector<vector<Point>> contours;
//	vector<Vec4i> hierarchy;
//	
//
//	VideoCapture capture(CV_CAP_ANY);
//
//	capture.set(CV_CAP_PROP_FPS, 60);
//	capture.set(CV_CAP_PROP_FRAME_HEIGHT, 400);
//	capture.set(CV_CAP_PROP_FRAME_WIDTH, 400);
//	
//	if (!capture.isOpened())
//	{
//		fprintf(stderr, "ERROR: No camera detected\n");
//		getchar();
//		return -1;
//	}
//
//	//namedWindow("Camera", CV_WINDOW_AUTOSIZE);
//	namedWindow("Contours", CV_WINDOW_AUTOSIZE);
//	
//	ofstream timefile;
//	timefile.open("timedata.txt");
//
//	StartCounter();
//	while (1)
//	{
//		Mat frame, frameEdges;
//
//		timefile << GetCounter() << "\n";
//		StartCounter();
//		capture >> frame;
//		if (frame.empty())
//		{
//			fprintf(stderr, "ERROR: Frame is null\n");
//			getchar();
//			break;
//		}
//
//
//		// Convert the frame to grayscale
//		cvtColor(frame, frame, CV_BGR2GRAY);
//
//		//imshow("Camera", frame);
//
//		// Detect edges and find contours
//		Canny(frame, frameEdges, lowThreshold, lowThreshold*thresholdRatio, kernelSize);
//		findContours(frameEdges, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, Point(0, 0));
//
//		// Draw contours
//		Mat drawing = Mat::zeros(frameEdges.size(), CV_8UC3);
//		for (int i = 0; i< contours.size(); i++)
//		{
//			Scalar color = Scalar(255, 255, 255);
//			drawContours(drawing, contours, i, color, 1, 8, hierarchy, 0, Point());
//		}
//
//		imshow("Contours", drawing);
//
//		char c = cvWaitKey(1);
//		if (c == 27) break;
//
//
//	}
//
//	//cvReleaseCapture(&capture);
//	capture.release();
//	destroyWindow("Camera");
//	timefile.close();
//	return 0;
//}