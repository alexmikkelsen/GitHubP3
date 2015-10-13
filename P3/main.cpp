#include "opencv2/core/core.hpp" 
#include "opencv2/highgui/highgui.hpp" 
#include "opencv2/imgproc/imgproc.hpp"


using namespace std;
using namespace cv;

int main() {
	Mat image, image2, image3, image4, image5;
	Mat element = Mat::ones(10, 10, CV_8UC1);
	//vector<Mat> channels;
	int value;

	VideoCapture cap(0);
	namedWindow("Webcam");

	waitKey(200);
	while (true){
		cap >> image;

		cvtColor(image, image2, CV_BGR2HSV);

		//split(image2, channels);

		// cv::threshold(channels[0],channels[0],40,80,CV_THRESH_BINARY);
		//cv::threshold(channels[1],channels[1],0.23,0.68,CV_THRESH_BINARY);

		GaussianBlur(image2, image3, Size(5, 5), 0, 0);
		inRange(image3, Scalar(0, 30, 60, 0), Scalar(20, 150, 255, 0), image4);

		erode(image4, image4, element);
		dilate(image4, image4, element);


		// cv::threshold(image2,image2,120,255,CV_THRESH_BINARY);

		//merge(channels,image3);

		imshow("Webcam", image4);
		waitKey(1); // Wait for a key press to show another image
	}
}