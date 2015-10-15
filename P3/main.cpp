#include "opencv2/core/core.hpp" 
#include "opencv2/highgui/highgui.hpp" 
#include "opencv2/imgproc/imgproc.hpp"


using namespace std;
using namespace cv;

int main() {
	Mat image, image2, image3, image4, image5;
	Mat element = Mat::ones(10, 10, CV_8UC1);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	VideoCapture cap(0);
	namedWindow("Webcam");

	waitKey(200);
	while (true){
		cap >> image;

		cvtColor(image, image2, CV_BGR2YCrCb);

		GaussianBlur(image2, image3, Size(5, 5), 0, 0);
		inRange(image3, Scalar(20, 140, 100, 0), Scalar(100, 200, 255, 0), image4);

		erode(image4, image4, element);
		dilate(image4, image4, element);

		findContours(image4, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

		/// Draw contours
		Mat drawing = Mat::zeros(image.size(), CV_8UC3);
		for (int i = 0; i< contours.size(); i++)
		{
			Scalar color(255, 255, 255);
			drawContours(drawing, contours, i, color, 2, 8, hierarchy);
		}

		imshow("Webcam", drawing);
		waitKey(1); // Wait for a key press to show another image
	}
}