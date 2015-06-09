#include <stdio.h>
#include <iostream>
#include <fstream>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

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
	int lowThreshold = 10;
	int thresholdRatio = 10;
	int kernelSize = 3;

	vector<vector<Point>> contours, template_contours;
	vector<Vec4i> hierarchy;
	

	VideoCapture capture(CV_CAP_ANY);

	capture.set(CV_CAP_PROP_FPS, 240);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, 200);
	capture.set(CV_CAP_PROP_FRAME_WIDTH, 200);

	if (!capture.isOpened())
	{
		fprintf(stderr, "ERROR: No camera detected\n");
		getchar();
		return -1;
	}

	//namedWindow("Camera", CV_WINDOW_AUTOSIZE);
	namedWindow("Contours", CV_WINDOW_AUTOSIZE);
	
	ofstream timefile;
	timefile.open("timedata.txt");

	StartCounter();
	bool show_feed = true;
	int total_frames = 1000;
	while (total_frames > 0)
	{
		Mat frame, frameEdges;

		timefile << GetCounter() << "\n";
		StartCounter();
		capture >> frame;
		if (frame.empty())
		{
			fprintf(stderr, "ERROR: Frame is null\n");
			getchar();
			break;
		}

		// Convert the frame to grayscale
		cvtColor(frame, frame, CV_BGR2GRAY);

		blur(frame, frame, Size(10, 10));

		threshold(frame, frame, 128, 255, CV_THRESH_BINARY);
		//imshow("Camera", frame);

		// Detect edges and find contours
		Canny(frame, frameEdges, lowThreshold, lowThreshold*thresholdRatio, kernelSize);
		contours.clear();
		findContours(frameEdges, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, Point(0, 0));

		// Find largest respective contours
		int largest_area = 0;
		int largest_contour_index_frame = -1;
		int largest_contour_index_template = -1;
		double area = 0;
		vector<Point> largest_contour;
		// Frame
		for (int i = 0; i < contours.size(); i++)
		{
			area = contourArea(contours[i], false);
			if (area > largest_area)
			{
				largest_area = area;
				largest_contour_index_frame = i;
				largest_contour = contours[i];
			}
		}
		// Template
		for (int i = 0; i < template_contours.size(); i++)
		{
			double area = contourArea(template_contours[i], false);
			if (area > largest_area)
			{
				largest_area = area;
				largest_contour_index_template = i;
			}
		}

		// Find minimum bounding rectangle and ellipse(rotated)
		RotatedRect boundingRect;
		RotatedRect boundingEllipse;
		if (area > 0)
		{
			boundingRect = minAreaRect(Mat(largest_contour));
			boundingEllipse = fitEllipse(Mat(largest_contour));
		}

		// Draw contours
		Mat drawing = Mat::zeros(frameEdges.size(), CV_8UC3);

		// Draw every contour
		/// for (int i = 0; i< contours.size(); i++)
		/// {
		/// 	Scalar color = Scalar(255, 255, 255);
		/// 	drawContours(drawing, contours, i, color, 1, 8, hierarchy, 0, Point());
		/// }

		// Draw one contour
		if (!contours.empty())
		{
			Scalar color_white = Scalar(255, 255, 255);
			Scalar color_red = Scalar(255, 0, 0);
			Scalar color_green = Scalar(0, 255, 0);
			drawContours(drawing, contours, largest_contour_index_frame, color_white, CV_FILLED, 8, hierarchy, 0, Point());
			ellipse(drawing, boundingEllipse, color_red, 2, 8);
			Point2f rect_points[4];
			boundingRect.points(rect_points);
			for (int i = 0; i < 4; i++)
				line(drawing, rect_points[i], rect_points[(i + 1) % 4], color_green, 1, 8);
		}


		if (show_feed == true)
		{
			imshow("Contours", drawing);
			char c = cvWaitKey(1);
			if (c == 27) break;
		}
		else
		{
			total_frames--;
		}


	}

	//cvReleaseCapture(&capture);
	capture.release();
	destroyWindow("Contours");
	timefile.close();
	return 0;
}