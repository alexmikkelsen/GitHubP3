#include "opencv2/core/core.hpp" 
#include "opencv2/highgui/highgui.hpp" 
#include "opencv2/imgproc/imgproc.hpp"
#include <math.h>
#include <windows.h>
#include <chrono>
#include <iostream>

using namespace std;
using namespace std::chrono;
using namespace cv;

int H_MIN = 35; //Ideal value = 35
int H_MAX = 82; //Ideal value = 82
int S_MIN = 40; //Ideal value = 40
int S_MAX = 255;
int V_MIN = 0;
int V_MAX = 255;



//void on_trackbar(int, void*){
//
//}
//
//void trackbars(){
//	namedWindow("Trackbars", 0);
//
//	char TrackbarName[50];
//	sprintf(TrackbarName, "H_MIN", H_MIN);
//	sprintf(TrackbarName, "H_MAX", H_MAX);
//	sprintf(TrackbarName, "S_MIN", S_MIN);
//	sprintf(TrackbarName, "S_MAX", S_MAX);
//	sprintf(TrackbarName, "V_MIN", V_MIN);
//	sprintf(TrackbarName, "V_MAX", V_MAX);
//
//	createTrackbar("H_MIN", "Trackbars", &H_MIN, H_MAX, on_trackbar);
//	createTrackbar("H_MAX", "Trackbars", &H_MAX, H_MAX, on_trackbar);
//	createTrackbar("S_MIN", "Trackbars", &S_MIN, S_MAX, on_trackbar);
//	createTrackbar("S_MAX", "Trackbars", &S_MAX, S_MAX, on_trackbar);
//	createTrackbar("V_MIN", "Trackbars", &V_MIN, V_MAX, on_trackbar);
//	createTrackbar("V_MAX", "Trackbars", &V_MAX, V_MAX, on_trackbar);
//}

double calc(Point i, Point j){
	double x_i = i.x;
	double y_i = i.y;

	double x_j = j.x;
	double y_j = j.y;

	double x_d = x_i - x_j;
	double y_d = y_i - y_j;

	return sqrt(x_d*x_d + y_d*y_d);
}

Mat threshold(Mat i){
	Mat img;

	// Creating a filter for erosion and dilation
	Mat element = Mat::ones(10, 10, CV_8UC1);

	// Convert input to HSV and store in 2nd matrix
	cvtColor(i, img, CV_BGR2HSV);

	// Threshold image2 based on the input from trackbars()
	inRange(img, Scalar(H_MIN, S_MIN, V_MIN, 0), Scalar(H_MAX, S_MAX, V_MAX, 0), img);

	// Show thresholded image
	imshow("Thresholding", img);

	// "Opening" the thresholded picture
	erode(img, img, element);
	dilate(img, img, element);

	return img;
}

int main() {
	// Creating two matrices to store the images in
	Mat image0, image;

	// Get input from webcam
	VideoCapture cap(0);

	// Call the trackbars
	//trackbars();

	bool openHand;

	while (true){
		
		//auto duration = 0;

		//String latency;

		//high_resolution_clock::time_point t1 = high_resolution_clock::now();
		
		// Store webcam input in image matrix
		cap >> image0;

		resize(image0, image, Size(), 0.5, 0.5, INTER_AREA);

		Mat image2 = threshold(image);

		// findContours() gets these two outputs
		vector<vector<Point> > contours; // Used to store output contours from the findContours function
		//vector<Vec4i> hierarchy; // Used to store the hierarchy of contours. Ex. Parent/Child

		/*
		void findContours(InputOutputArray image, OutputArrayOfArrays contours, OutputArray hierarchy, int mode, int method, Point offset=Point())
		*/
		findContours(image2, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

		/// Find the convex hull object for each contour, and find convexity defects
		vector<vector<Point> >hull(contours.size()); // Output for convexHull() stored in a vector of points
		vector<vector<int> > convPoints(contours.size()); // Output for convexHull() stored in integer vector of indices

		vector<vector<Vec4i> > convexDefects(contours.size()); // Output vector for convexityDefects(), stored in a 4-element integer vector

		for (int i = 0; i < contours.size(); i++)
		{
			/*
			void convexHull(InputArray points, OutputArray hull, bool clockwise=false, bool returnPoints=true )
			*/
			convexHull(contours[i], hull[i], false);
			convexHull(contours[i], convPoints[i], false);
			if (convPoints[i].size() > 3){ // If there are more than 3 convPoints in the current contour, Run
				/*
				void convexityDefects(InputArray contour, InputArray convexhull, OutputArray convexityDefects)
				*/
				convexityDefects(contours[i], convPoints[i], convexDefects[i]);
			}
		}

		// Locate the biggest contour
		int contourIndex = 0;
		int area = 0;
		for (int i = 0; i < contours.size(); i++) // Run through all contours, one-by-one
		{
			double a = contourArea(contours[i]); // Store the size of the current contour in a double
			if (a > area)
			{
				area = a;					// Set "area" to the current contour's area size
				contourIndex = i;			// Set the index to be equal to the current contour
			}
		}

		/// If the biggest contour area is larger than 200, draw it
		if (area > 500){
			/*
			void drawContours(InputOutputArray image, InputArrayOfArrays contours, int contourIdx, const Scalar& color, int thickness, int lineType, InputArray hierarchy=noArray(), int maxLevel, Point offset=Point() )
			*/
			//drawContours(image, hull, contourIndex, 255, 3, 8, vector<Vec4i>(), 0, Point());

			//Bounding Box
			Rect boundRect = boundingRect(contours[contourIndex]);
			//rectangle(image, boundRect.tl(), boundRect.br(), Scalar(255, 255, 255), 2, 8, 0);

			int fingers = 0;
			int extraFinger;

			for (int i = 0; i < convexDefects[contourIndex].size(); i++){
				const Vec4i& vec = convexDefects[contourIndex][i];
				float depth = vec[3] / 256; // Find the points furthest from the convex hull
				if (depth > 20) // If the distance between the convex hull and the point is big enough, Run
				{
					int start = vec[0]; // Starting point of the defect on the contour
					Point startPt(contours[contourIndex][start]); // Assign a point to the start location

					int end = vec[1]; // Ending point of the defect on the contour
					Point endPt(contours[contourIndex][end]);// Assign a point to the end location

					int furthest = vec[2]; // Point of the defect, furthest away from the convex hull
					Point furthestPt(contours[contourIndex][furthest]); // Assign a point to the furthest away location

					double a = calc(startPt, endPt);
					double b = calc(startPt, furthestPt);
					double c = calc(endPt, furthestPt);

					int angle = ((acos((b*b + c*c - a*a) / (2 * b*c))) * 180) / 3.1415;

					openHand = false;

					if (angle > 10){
						openHand = true;
						fingers += 1;
						//extraFinger = fingers + 1;

						//line(image, startPt, endPt, Scalar(0, 0, 255), 1); // Draw line between start and end
						//line(image, startPt, furthestPt, Scalar(0, 0, 255), 1); // Draw line between start and furthest
						//line(image, endPt, furthestPt, Scalar(0, 0, 255), 1); // Draw line between end and furthest
						//circle(image, furthestPt, 4, Scalar(255, 0, 0), 2); // Draw a little circle at the furthest point

						//if (extraFinger == 2){
						//	String s = to_string(fingers);
						//	putText(image, s, startPt, FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 0, 255), 3, LINE_8, false);
						//}
						//String t = to_string(extraFinger);
						//putText(image, t, endPt, FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 0, 255), 3, LINE_8, false);
					}
				}
			}
			//Showing releationship between height and width
			float w = (float)boundRect.width;
			float h = (float)boundRect.height;
			float ratio = w / h;

			/*ostringstream convert;
			convert << ratio;
			String t = convert.str();*/
			//putText(image, "Here " + t, Point(50, 50), FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 0, 255), 3, LINE_8, false);

			char stance = 'U';

			//IF's with bounding box
			//high_resolution_clock::time_point t1 = high_resolution_clock::now();
			if ((openHand == false || fingers < 2) && ratio > 0.6 && ratio < 1.2) {
				putText(image0, "Fist", Point(200, 200), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 255, 255), 3, LINE_8, false);
				stance = 'A';
				//latency = "Fist";
			}
			else if (ratio < 0.4){
				putText(image0, "Karate", Point(200, 200), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 255, 255), 3, LINE_8, false);
				stance = 'B';
				//latency = "Karate";
			}
			else if (fingers > 2){
				putText(image0, "Open hand", Point(200, 200), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 255, 255), 3, LINE_8, false);
				stance = 'C';
				//latency = "Open hand";
			}
			else if (fingers == 1 || fingers == 2){
				putText(image0, "Peace", Point(200, 200), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 255, 255), 3, LINE_8, false);
				stance = 'D';
				//latency = "Peace";
			}
			//high_resolution_clock::time_point t2 = high_resolution_clock::now();
			//duration = duration_cast<milliseconds>(t2 - t1).count();

			static bool resB = false, resC = false, resD = false;

			switch (stance){
			case 'A':
				if (resB == true || resC == true || resD == true){
					resB = false;
					resC = false;
					resD = false;
				}
				break;
			case 'B':
				if (resB == false){
					keybd_event(VK_LCONTROL, 0x9d, 0, 0);
					keybd_event(VK_LCONTROL, 0x9d, KEYEVENTF_KEYUP, 0);
					resB = true;
				}
				break;
			case 'C':
				if (resC == false){
					keybd_event(VK_SPACE, 0xb9, 0, 0);
					Sleep(150);
					keybd_event(VK_SPACE, 0xb9, KEYEVENTF_KEYUP, 0);
					Sleep(350);
					keybd_event(VK_SPACE, 0xb9, 0, 0);
					Sleep(150);
					keybd_event(VK_SPACE, 0xb9, KEYEVENTF_KEYUP, 0);
					resC = true;
				}
				break;
			case 'D':
				if (resD == false){
					keybd_event(VK_SPACE, 0xb9, 0, 0);
					Sleep(80);
					keybd_event(VK_SPACE, 0xb9, KEYEVENTF_KEYUP, 0);
					resD = true;
				}
				break;
			}
		}
		imshow("Webcam", image0);
		waitKey(1);

		//high_resolution_clock::time_point t2 = high_resolution_clock::now();

		//duration = duration_cast<milliseconds>(t2 - t1).count();

		/*if (duration > 0){
			cout << "The duration is: " << duration << " ms for hand sign: " << latency << endl;
		}*/
	}
}