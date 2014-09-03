#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <gl\gl.h>		
#include <gl\glu.h>	

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"

using namespace cv;
using namespace std;

Mat src; Mat src_gray;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);

/** @function thresh_callback */
void thresh_callback(int, void*)
{
	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	/// Detect edges using canny
	Canny(src_gray, canny_output, thresh, thresh * 2, 3);

	//threshold(src_gray, canny_output, thresh, thresh * 2, 0);
	/// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));


	/// Draw contours
	Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
	Scalar color = Scalar(255, 255, 255);
	for (int i = 0; i< contours.size(); i++)
	{
		
		Moments m = moments(contours[i]);
		double p = arcLength(contours[i], true);
		printf("%f -> %d \n", p, i);
		if (p > 300)
			drawContours(drawing, contours, i, color, 1, 8, hierarchy, 0, Point());
	}
	/// Show in a window
	//imshow("CONTOUR", drawing);
	imshow("CONTOUR", canny_output);
}

int _tmain(int argc, _TCHAR* argv[])
{
	printf("==============================================================================\n");
	printf("== CONTOUR DETECTION                                TARAMBOLA DEVELOPMENT ==\n");
	printf("==============================================================================\n\n");

	//»»»»»»»»»»»»»» IMAGE
	 src = imread( "fotografia.JPG" );

	 /// Convert image to gray and blur it
	 cvtColor(src, src_gray, CV_BGR2GRAY);
	 blur(src_gray, src_gray, Size(3, 3));

	//************ WINDOW **************
	namedWindow("CONTOUR", CV_WINDOW_AUTOSIZE);

	imshow("CONTOUR", src);

	createTrackbar(" Canny thresh:", "CONTOUR", &thresh, max_thresh, thresh_callback);
	thresh_callback(0, 0);

	while (1)
	{
		Sleep(2);


		int key = waitKey(10);
		if (key == 27) break;
	}
	return 0;
}
