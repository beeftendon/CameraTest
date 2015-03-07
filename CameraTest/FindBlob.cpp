#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>

using namespace cv;
using namespace std;

/// Global variables

/**
* @function CannyThreshold
* @brief Trackbar callback - Canny thresholds input with a ratio 1:3
*/
/*
void CannyThreshold(int, void*)
{
	
}*/


/** @function main */
int main(int argc, char** argv)
{
	Mat src_img, src_ref, src_gray;
	Mat dst, detected_edges;

	int edgeThresh = 1;
	int lowThreshold = 10;
	int const max_lowThreshold = 1000;
	int ratio = 3;
	int kernel_size = 3;
	char* window_name = "Edge Map";

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	RNG rng(12345);

	/// Load an image
	src_img = imread(argv[1]);
	//src_ref = imread(argv[2]);

	if (!src_img.data)
	{
		return -1;
	}

	/// Create a matrix of the same type and size as src_img (for dst)
	dst.create(src_img.size(), src_img.type());

	/// Convert the image to grayscale
	cvtColor(src_img, src_img, CV_BGR2GRAY);

	/// Create a window
	namedWindow(window_name, CV_WINDOW_AUTOSIZE);

	/// Create a Trackbar for user to enter threshold
	//createTrackbar("Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold);

	/// Show the image
	//CannyThreshold(0, 0);

	blur(src_img, detected_edges, Size(3, 3));
	/// Canny detector
	Canny(detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size);
	findContours(detected_edges, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, Point(0, 0));

	/// Draw contours
	Mat drawing = Mat::zeros(detected_edges.size(), CV_8UC3);
	for (int i = 0; i< contours.size(); i++)
	{
		Scalar color = Scalar(255, 255, 255);
		drawContours(drawing, contours, i, color, 1, 8, hierarchy, 0, Point());
	}

	/// Using Canny's output as a mask, we display our result
	dst = Scalar::all(0);

	src_img.copyTo(dst, detected_edges);
	imshow(window_name, drawing);

	/// Wait until user exit program by pressing a key
	waitKey(0);

	return 0;
}