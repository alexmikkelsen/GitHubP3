#include "opencv2/core/core.hpp" 
#include "opencv2/highgui/highgui.hpp" 
#include "opencv2/imgproc/imgproc.hpp"
#include <math.h>
#include <windows.h>

using namespace std;
using namespace cv;

int H_MIN = 35; //Ideal value = 35
int H_MAX = 82; //Ideal value = 82
int S_MIN = 40; //Ideal value = 40
int S_MAX = 255;
int V_MIN = 0;
int V_MAX = 255;

void on_trackbar(int, void*){

}

void trackbars(){
	namedWindow("Trackbars", 0);

	char TrackbarName[50];
	sprintf(TrackbarName, "H_MIN", H_MIN);
	sprintf(TrackbarName, "H_MAX", H_MAX);
	sprintf(TrackbarName, "S_MIN", S_MIN);
	sprintf(TrackbarName, "S_MAX", S_MAX);
	sprintf(TrackbarName, "V_MIN", V_MIN);
	sprintf(TrackbarName, "V_MAX", V_MAX);

	createTrackbar("H_MIN", "Trackbars", &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", "Trackbars", &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", "Trackbars", &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", "Trackbars", &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", "Trackbars", &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", "Trackbars", &V_MAX, V_MAX, on_trackbar);
}

double calc(Point i, Point j){
	double x_i = i.x;
	double y_i = i.y;

	double x_j = j.x;
	double y_j = j.y;

	double x_d = x_i - x_j;
	double y_d = y_i - y_j;

	return sqrt(x_d*x_d + y_d*y_d);
}

int main() {
	// Creating two matrices to store the images in
	Mat image, image2;

	// Creating a filter for erosion and dilation
	Mat element = Mat::ones(10, 10, CV_8UC1);

	// Get input from webcam
	VideoCapture cap(0);
	namedWindow("Webcam");

	// Call the trackbars
	trackbars();

	bool openHand;

	while (true){
		// Store webcam input in image matrix
		cap >> image;

		// Convert input to HSV and store in 2nd matrix
		cvtColor(image, image2, CV_BGR2HSV);

		// Threshold image2 based on the input from trackbars()
		inRange(image2, Scalar(H_MIN, S_MIN, V_MIN, 0), Scalar(H_MAX, S_MAX, V_MAX, 0), image2);

		// Show thresholded image
		imshow("Thresholding", image2);

		// "Opening" the thresholded picture
		erode(image2, image2, element);
		dilate(image2, image2, element);

		// findContours() gets these two outputs
		vector<vector<Point> > contours; // Used to store output contours from the findContours function
		vector<Vec4i> hierarchy; // Used to store the hierarchy of contours. Ex. Parent/Child

		/*
		void findContours(InputOutputArray image, OutputArrayOfArrays contours, OutputArray hierarchy, int mode, int method, Point offset=Point())
		*/
		findContours(image2, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

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
			drawContours(image, hull, contourIndex, 255, 3, 8, vector<Vec4i>(), 0, Point());

			//Bounding Box
			Rect boundRect = boundingRect(contours[contourIndex]);
			rectangle(image, boundRect.tl(), boundRect.br(), Scalar(255, 255, 255), 2, 8, 0);

			int fingers = 0;
			int extraFinger;

			for (int i = 0; i < convexDefects[contourIndex].size(); i++){
				const Vec4i& vec = convexDefects[contourIndex][i];
				float depth = vec[3] / 256; // Find the points furthest from the convex hull
				if (depth > 40) // If the distance between the convex hull and the point is big enough, Run
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
						extraFinger = fingers + 1;

						line(image, startPt, endPt, Scalar(0, 0, 255), 1); // Draw line between start and end
						line(image, startPt, furthestPt, Scalar(0, 0, 255), 1); // Draw line between start and furthest
						line(image, endPt, furthestPt, Scalar(0, 0, 255), 1); // Draw line between end and furthest
						circle(image, furthestPt, 4, Scalar(255, 0, 0), 2); // Draw a little circle at the furthest point

						if (extraFinger == 2){
							String s = to_string(fingers);
							putText(image, s, startPt, FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 0, 255), 3, LINE_8, false);
						}
						String t = to_string(extraFinger);
						putText(image, t, endPt, FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 0, 255), 3, LINE_8, false);
					}
				}
			}
			//Showing releationship between height and width
			float w = (float)boundRect.width;
			float h = (float)boundRect.height;
			float lol = w / h;
			String t;
			ostringstream convert;
			convert << lol;
			t = convert.str();
			putText(image, "Here " + t, Point(50, 50), FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 0, 255), 3, LINE_8, false);

			char stance = 'U';

			//IF's with bounding box
			if ((openHand == false || fingers < 2) && w / h > 0.6 && w / h < 1.2) {
				putText(image, "Fist", Point(200, 200), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 255, 255), 3, LINE_8, false);
				stance = 'A';
			}
			else if (w / h < 0.4){
				putText(image, "Karate", Point(200, 200), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 255, 255), 3, LINE_8, false);
				stance = 'B';
			}
			else if (fingers > 2){
				putText(image, "Open hand", Point(200, 200), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 255, 255), 3, LINE_8, false);
				stance = 'C';
			}
			else if (fingers == 1 || fingers == 2){
				putText(image, "Peace", Point(200, 200), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 255, 255), 3, LINE_8, false);
				stance = 'D';
			}

			switch (stance){
			case 'A':
				keybd_event(VkKeyScan('A'), 0x9e, 0, 0);
				keybd_event(VkKeyScan('A'), 0x9e, KEYEVENTF_KEYUP, 0);
				break;
			case 'B':
				keybd_event(VkKeyScan('B'), 0xb0, 0, 0);
				keybd_event(VkKeyScan('B'), 0xb0, KEYEVENTF_KEYUP, 0);
				break;
			case 'C':
				keybd_event(VkKeyScan('C'), 0xae, 0, 0);
				keybd_event(VkKeyScan('C'), 0xae, KEYEVENTF_KEYUP, 0);
				break;
			case 'D':
				keybd_event(VkKeyScan('D'), 0xa0, 0, 0);
				keybd_event(VkKeyScan('D'), 0xa0, KEYEVENTF_KEYUP, 0);
				break;
			}
		}
		imshow("Webcam", image);
		waitKey(1);
	}
}