#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <gl\gl.h>		
#include <gl\glu.h>	
#include <iostream>


#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"

using namespace cv;
using namespace std;

Mat src, src_gray, src_canny;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);
float nearD = 1000;
Point near1, near2;


/** @function thresh_callback */
void thresh_callback(int, void*)
{
	Mat canny_output;
	Canny(src_gray, canny_output, thresh, thresh * 2, 3);

	vector<Vec2f> lines;
	// detect lines
	HoughLines(canny_output, lines, 1, CV_PI / 180, 150, 0, 0);

	Mat clone = src.clone();
	Point c = Point(400,400);
	
	
	// draw line
	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];
		Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
	//	cout << "PT1(" << pt1.x << ", " << pt1.y << ") , PT2(" << pt2.x << ", " << pt2.y << ") \n";
		Point media = Point((pt2.x + pt1.x)/2, (pt2.y + pt1.y)/2);
		float d = sqrt((media.x - c.x)*(media.x - c.x) + (media.y - c.y)*(media.y - c.y));
	//	cout << d << "\n";
	/*	if (d < nearD && i >3)
		{
			clone = src.clone();
			cout << "near=" << nearD << ", d=" << d << "\n";
			nearD = d;
			near1 = pt1;
			near2 = pt2;
			line(clone, pt1, pt2, Scalar(0, 0, 255), 1, CV_AA);
		}
		else*/
		if ((theta <0.05 && theta >-0.05) || (theta>1.5 && theta<1.6))
		{
			cout << theta << "\n";
			line(clone, pt1, pt2, Scalar(0, 0, 255), 1, CV_AA);
		}
		
	}
	
	Point c1 = Point(380, 400);
	Point c2 = Point(420,400);
	Point c3 = Point(400, 380);
	Point c4 = Point(400, 420);
	line(clone, c1, c2, Scalar(100, 0, 255), 1, CV_AA);
	line(clone, c3, c4, Scalar(100, 0, 255), 1, CV_AA);

	imshow("CONTOUR", canny_output);
	imshow("ORIGINAL", clone);
}

int _tmain(int argc, _TCHAR* argv[])
{
	printf("==============================================================================\n");
	printf("== CONTOUR DETECTION                                TARAMBOLA DEVELOPMENT ==\n");
	printf("==============================================================================\n\n");

	//»»»»»»»»»»»»»» IMAGE
	 src_gray = imread( "fotografia.JPG", 0);
	 src = imread("fotografia.JPG");

	 /// Convert image to gray and blur it
	
	// cvtColor(src, src_gray, CV_BGR2GRAY);
	// blur(src_gray, src_gray, Size(3, 3));

	 

	//************ WINDOW **************
	namedWindow("CONTOUR", CV_WINDOW_AUTOSIZE);
	namedWindow("ORIGINAL", CV_WINDOW_AUTOSIZE);

	

	createTrackbar(" Canny thresh:", "CONTOUR", &thresh, max_thresh, thresh_callback);
	thresh_callback(0, 0);

	cout << "width=" << src.size().width << "\n";

	while (1)
	{
		Sleep(2);


		int key = waitKey(10);
		if (key == 27) break;
	}
	return 0;
}
