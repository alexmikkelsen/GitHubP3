#include "opencv2/core/core.hpp" 
#include "opencv2/highgui/highgui.hpp" 
#include "opencv2/imgproc/imgproc.hpp"


using namespace std;
using namespace cv;

int main() {
	Mat image, image2, image3, image4, image5, image03;
	Mat element = Mat::ones(10, 10, CV_8UC1);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	VideoCapture cap(0);
	namedWindow("Webcam");

	waitKey(200);
	while (true){
		cap >> image;

		cvtColor(image, image2, CV_BGR2HSV);

		//GaussianBlur(image2, image3, Size(3, 3), 0, 0);


		inRange(image2, Scalar(20, 100, 50, 0), Scalar(100, 200, 150, 0), image3);

		medianBlur(image3, image4, 3);

		erode(image4, image4, element);
		dilate(image4, image4, element);

		//imshow("Webfdcam", image4);

		findContours(image4, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

		/// Find the convex hull object for each contour
		vector<vector<Point> >hull(contours.size());
		//vector<Vec4i> convexityDefects;
		for (int i = 0; i < contours.size(); i++)
		{
			convexHull(Mat(contours[i]), hull[i], false);
			//convexityDefects(Mat(contours[i]), hull[i], convexityDefects);
		}

		/// Draw contours
		Mat drawing = Mat::zeros(image.size(), CV_8UC3);
		for (int i = 0; i< contours.size(); i++)
		{
			Scalar color(255, 255, 255);
			drawContours(drawing, contours, i, color, CV_FILLED, 8, hierarchy, 0, Point());
			drawContours(drawing, hull, i, color, 3, 8, vector<Vec4i>(), 0, Point());
		}

		imshow("Webcam", drawing);
		waitKey(1); // Wait for a key press to show another image
	}
}