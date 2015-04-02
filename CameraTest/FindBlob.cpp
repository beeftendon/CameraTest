#include <stdio.h>
#include <iostream>
#include <fstream>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/gpu/gpu.hpp"

using namespace cv;
using namespace std;

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

/** @function main */
int main(int argc, char** argv)
{
	Mat img_object = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
	img_object = img_object > 160;
	blur(img_object, img_object, Size(3, 3));
	//Mat img_scene = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);
	Mat img_scene;
	Mat img_scene_cap;
	VideoCapture capture(CV_CAP_ANY);
	capture.set(CV_CAP_PROP_FPS, 60);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, 200);
	capture.set(CV_CAP_PROP_FRAME_WIDTH, 200);

	ofstream timefile;
	timefile.open("timedata.txt");


	//-- Step 1: Detect the keypoints using SURF Detector
	int minHessian = 1000;

	SurfFeatureDetector detector(minHessian);
	std::vector<KeyPoint> keypoints_object;
	detector.detect(img_object, keypoints_object);


	//-- Step 2: Calculate descriptors (feature vectors)
	SurfDescriptorExtractor extractor;
	Mat descriptors_object;
	if (keypoints_object.empty())
	{
		cvError(0, "MatchFinder", "Object keypoints empty", __FILE__, __LINE__);
	}
	extractor.compute(img_object, keypoints_object, descriptors_object);

	while (true)
	{
		StartCounter();

		capture >> img_scene;
		cvtColor(img_scene, img_scene, CV_BGR2GRAY);
		img_scene = img_scene > 160;
		blur(img_scene, img_scene, Size(3, 3));

		if (!img_object.data) // || !img_scene.data)
		{
			std::cout << " --(!) Error reading images " << std::endl; return -1;
		}
		
		//cvtColor(img_scene_cap, img_scene, CV_BayerRG2GRAY);


		std::vector<KeyPoint> keypoints_scene;

		detector.detect(img_scene, keypoints_scene);


		Mat descriptors_scene;

		if (keypoints_object.empty())
		{
			cvError(0, "MatchFinder", "Object keypoints empty", __FILE__, __LINE__);
		}
		if (keypoints_scene.empty())
		{
			GetCounter();
			char c = waitKey(1);
			if (c == 27) return 0;
			continue;
			//cvError(0, "MatchFinder", "Scene keypoints empty", __FILE__, __LINE__);
		}

		extractor.compute(img_scene, keypoints_scene, descriptors_scene);

		if (descriptors_object.empty())
		{
			cvError(0, "MatchFinder", "Object descriptor empty", __FILE__, __LINE__);
		}
		if (descriptors_scene.empty())
		{
			//cvError(0, "MatchFinder", "Scene descriptor empty", __FILE__, __LINE__);
		}

		//-- Step 3: Matching descriptor vectors using FLANN matcher
		FlannBasedMatcher matcher;
		std::vector< DMatch > matches;
		matcher.match(descriptors_object, descriptors_scene, matches);

		double max_dist = 0; double min_dist = 99;

		//-- Quick calculation of max and min distances between keypoints
		for (int i = 0; i < descriptors_object.rows; i++)
		{
			double dist = matches[i].distance;
			if (dist < min_dist) min_dist = dist;
			if (dist > max_dist) max_dist = dist;
		}

		//printf("-- Max dist : %f \n", max_dist);
		//printf("-- Min dist : %f \n", min_dist);

		//-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
		std::vector< DMatch > good_matches;

		for (int i = 0; i < descriptors_object.rows; i++)
		{
			if (matches[i].distance < 3 * min_dist)
			{
				good_matches.push_back(matches[i]);
			}
		}
		
		Mat img_matches;
		drawMatches(img_object, keypoints_object, img_scene, keypoints_scene,
			good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
			vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

		//-- Localize the object
		std::vector<Point2f> obj;
		std::vector<Point2f> scene;

		for (int i = 0; i < good_matches.size(); i++)
		{
			//-- Get the keypoints from the good matches
			obj.push_back(keypoints_object[good_matches[i].queryIdx].pt);
			scene.push_back(keypoints_scene[good_matches[i].trainIdx].pt);
		}

		//-- Get the corners from the image_1 ( the object to be "detected" )
		std::vector<Point2f> obj_corners(4);
		std::vector<Point2f> scene_corners(4);
		Mat H;

		try
		{
			
			H = findHomography(obj, scene, CV_RANSAC);

		}
		catch (...)
		{
			GetCounter();
			char c = waitKey(1);
			if (c == 27) return 0;
			continue;
		}

		obj_corners[0] = cvPoint(0, 0); obj_corners[1] = cvPoint(img_object.cols, 0);
		obj_corners[2] = cvPoint(img_object.cols, img_object.rows); obj_corners[3] = cvPoint(0, img_object.rows);

		perspectiveTransform(obj_corners, scene_corners, H);

		//-- Draw lines between the corners (the mapped object in the scene - image_2 )
		line(img_matches, scene_corners[0] + Point2f(img_object.cols, 0), scene_corners[1] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
		line(img_matches, scene_corners[1] + Point2f(img_object.cols, 0), scene_corners[2] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
		line(img_matches, scene_corners[2] + Point2f(img_object.cols, 0), scene_corners[3] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
		line(img_matches, scene_corners[3] + Point2f(img_object.cols, 0), scene_corners[0] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);

		//-- Show detected matches
		//imshow("Good Matches & Object detection", img_matches);
		
		printf("-- Time : %f \n", GetCounter());

		char c = waitKey(1);
		if (c == 27) return 0;
	}

}
