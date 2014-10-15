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

#define PI 3.14159265;

Mat src, src_gray, src_canny;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);
float nearD = 1000;
Point near1, near2;
int imgWidth;
int imgHeight;
int thresh_corner = 200;
int alpha = 1; /**< Simple contrast control */
int beta = 100;  /**< Simple brightness control */
int blurValue = 1;



void detectCorners(int, void*)
{
	Mat dst, dst_norm, dst_norm_scaled;
	dst = Mat::zeros(src.size(), CV_32FC1);
	Mat clone = src.clone();
	Mat canny_output;
	Canny(src_gray, canny_output, thresh, thresh * 2, 3);

	int blockSize = 2;
	int apertureSize = 3;
	double k = 0.04;

	cornerHarris(canny_output, dst, blockSize, apertureSize, k, BORDER_DEFAULT);

	normalize(dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
	convertScaleAbs(dst_norm, dst_norm_scaled);

	for (int j = 0; j < dst_norm.rows; j++)
	{
		for (int i = 0; i < dst_norm.cols; i++)
		{
			if ((int)dst_norm.at<float>(j, i)>thresh_corner)
			{
				circle(clone, Point(i, j), 4, Scalar(0,0,255), 1, 8, 0);

			}
		}
	}
	imshow("ORIGINAL", clone);
}
void thresh_callback2(int, void*)
{
	int pixelCount = 0;
	for (int j = 0; j < src_canny.rows; j++)
	{
		for (int i = 0; i < src_canny.cols; i++)
		{
			if ((int)src_canny.at<uchar>(j, i) > 0)
			{
				pixelCount++;
			}
		}
	}
	int total = src_canny.rows*src_canny.cols;

	double perc = (pixelCount / total) * 100;

	string Result;          // string which will contain the result

	ostringstream convert;   // stream used for the conversion

	//convert << perc;      // insert the textual representation of 'Number' in the characters in the stream

//	Result = convert.str();

	cout << "PixelCount: " << pixelCount << " ,total: " << total  << "\n";
	cout << total << "\n";
}
/* function draw Gripper */
void drawGripper(Mat img, Point2f center, float angle)
{
	RotatedRect rect = RotatedRect(center, Size2f(200.0, 240.0), angle);

	Point2f vertices[4];
	rect.points(vertices);
	for (int i = 0; i < 4; i++)
		line(img, vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 0));
}
/** @function thresh_callback */
void thresh_callback(int, void*)
{
	Mat canny_output;
	Mat blured;
	Mat lap;
	if (blurValue != 0)
		blur(src_gray, blured, Size(blurValue, blurValue));
	else
		blured = src_gray;
	//Laplacian(blured, lap,CV_8S);

	blured.convertTo(lap, -1, alpha, beta);

	Canny(blured, canny_output, thresh, thresh * 2, 3);

	vector<Vec2f> lines;
	// detect lines
	HoughLines(canny_output, lines, 1, CV_PI / 180, 150, 0, 0);

	Mat clone = src.clone();
	Point c = Point(imgWidth/2,imgHeight/2);

	src_canny = canny_output;
	
	
	// draw line
	/*for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];
		Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		Point media = Point((pt2.x + pt1.x)/2, (pt2.y + pt1.y)/2);
		float d = sqrt((media.x - c.x)*(media.x - c.x) + (media.y - c.y)*(media.y - c.y));
		if ((theta <0.05 && theta >-0.05) || (theta>1.5 && theta<1.6))
		{
			cout << theta << "\n";
			line(clone, pt1, pt2, Scalar(0, 0, 255), 1, CV_AA);
		}
		
	}*/
	
	Point m1 = Point(imgWidth/2 - 20, imgHeight/2);
	Point m2 = Point(imgWidth/2+20, imgHeight/2);
	Point m3 = Point(imgWidth/2, imgHeight/2 - 20);
	Point m4 = Point(imgWidth/2, imgHeight/2 + 20);
	line(clone, m1, m2, Scalar(0, 255, 255), 1, CV_AA);
	line(clone, m3, m4, Scalar(0, 255, 255), 1, CV_AA);

	int rowM =0;
	int rowL = 0;
	int rowR = 0;
	int rowB = 0;

	int colL = 0;
	int colR = 0;

	//***************** LATERAIS
	for (int l = canny_output.rows / 2; l < canny_output.rows; l++)
	{
		if ((int)canny_output.at<uchar>(l, canny_output.cols / 2) > 0)
		{
			rowB = l;
			break;
		}
	}
	for (int j = canny_output.cols / 2; j > 0; j--)
	{
		if ((int)canny_output.at<uchar>(canny_output.rows / 2, j) > 0)
		{
			colL = j;
			break;
		}
	}
	for (int k = canny_output.cols / 2; k < canny_output.cols; k++)
	{
		if ((int)canny_output.at<uchar>(canny_output.rows / 2, k) > 0)
		{
			colR = k;
			break;
		}
	}
	int wR = canny_output.cols - colR;
	int wL = colL;
	int dif = (wR - wL) / 2;

	line(clone, Point(colL, imgHeight / 2 - 10), Point(colL, imgHeight / 2 + 10), Scalar(0, 0, 255), 1, CV_AA);
	line(clone, Point(colR, imgHeight / 2 - 10), Point(colR, imgHeight / 2 + 10), Scalar(0, 0, 255), 1, CV_AA);
	//***************** @@LATERAIS
	//***************** GOLA
	for (int i = canny_output.rows/2; i > 0; i--)
	{
		if ((int)canny_output.at<uchar>(i, (canny_output.cols / 2)-dif) > 0 && rowM==0)
		{
			rowM = i;
		}
		if ((int)canny_output.at<uchar>(i, (canny_output.cols / 2 - 20)-dif) > 0 && rowL == 0)
		{
			rowL = i;
		}
		if ((int)canny_output.at<uchar>(i, (canny_output.cols / 2 + 20)-dif) > 0 && rowR == 0)
		{
			rowR = i;
		}
	}



	line(clone, Point((imgWidth / 2 - 10) - dif, rowM), Point((imgWidth / 2 + 10) - dif, rowM), Scalar(100, 0, 255), 1, CV_AA);
	line(clone, Point((imgWidth / 2 - 25) - dif, rowL), Point((imgWidth / 2 - 15) - dif, rowL), Scalar(0, 255, 255), 1, CV_AA);
	line(clone, Point((imgWidth / 2 + 15) - dif, rowR), Point((imgWidth / 2 + 25) - dif, rowR), Scalar(0, 255, 255), 1, CV_AA);
	//line(clone, Point((imgWidth / 2 - 20)-dif, rowB), Point((imgWidth / 2 + 20)-dif, rowB), Scalar(100, 0, 255), 1, CV_AA);

	//******************@@GOLA

	//***************** ANGULO
	double ang, m, top, bottom, deg;

	top = (rowL - rowR);
	bottom = (((canny_output.cols / 2 - 20) - dif) - ((canny_output.cols / 2 + 20) - dif));

	m = top / bottom;

	ang = atan(m);

	deg = ang*(180 / 3.14159265 );

	//cout.precision(9);
	//cout << fixed << rowL << "-" << rowR << "/" << ((canny_output.cols / 2 - 20) - dif) << "-" << ((canny_output.cols / 2 + 20) - dif) << "=" << m << "<=> ang=" << deg << endl;
	//****************» @@ANGULO

	int hT = rowM;
	int hB = canny_output.rows - rowB;
	int difH = -rowM;// (hB - hT) / 2;

	int *rows = new int[3];
	rows[0] = rowL;
	rows[1] = rowM;
	rows[2] = rowR;
	int frst=0;

	for (int m = 0; m < sizeof(rows); m++)
	{
		if (rows[m]>frst)
			frst = rows[m];
	}
	difH = -frst;

	Point c1 = Point(imgWidth / 2 - 20 - dif , imgHeight / 2 - difH);
	Point c2 = Point(imgWidth / 2 + 20 - dif, imgHeight / 2 - difH);
	Point c3 = Point(imgWidth / 2 - dif, imgHeight / 2 - 20 - difH);
	Point c4 = Point(imgWidth / 2 - dif, imgHeight / 2 + 20 - difH);
	line(clone, c1, c2, Scalar(100, 0, 255), 1, CV_AA);
	line(clone, c3, c4, Scalar(100, 0, 255), 1, CV_AA);

	drawGripper(clone, Point2f(((imgWidth / 2) - dif), ((imgHeight / 2) - difH)), deg);


	imshow("CONTOUR", canny_output);
	imshow("ORIGINAL", clone);
}

int _tmain(int argc, _TCHAR* argv[])
{
	printf("==============================================================================\n");
	printf("== CONTOUR DETECTION                                TARAMBOLA DEVELOPMENT ==\n");
	printf("==============================================================================\n\n");

	//************** CAMERA
	VideoCapture cap(0);
	if (!cap.isOpened())
	{
		cout << "CAMERA NOT CONNECTED!";
		return(-1);
	}
	
	
	//»»»»»»»»»»»»»» IMAGE
	// src_gray = imread( "red2.jpg", 0);
	// src = imread("red2.jpg");

	Sleep(2);
	//cap.set(CV_CAP_PROP_MODE, -2);
	Sleep(2);
	//cap.set(CV_CAP_PROP_FPS, 1);
	Sleep(2);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 320);
	Sleep(2);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
	Sleep(2);
	cap.set(CV_CAP_PROP_BRIGHTNESS, 100.0);
	Sleep(2);
	cap.set(CV_CAP_PROP_CONTRAST, 10.0);
	Sleep(2);
	cap.set(CV_CAP_PROP_EXPOSURE, 10.0);

	cap >> src;
	cvtColor(src, src_gray, CV_BGR2GRAY);

	cout << cap.get(CV_CAP_PROP_MODE);


//	 resize(src, src, Size(), 0.5, 0.5, INTER_CUBIC);
//	 resize(src_gray, src_gray, Size(), 0.5, 0.5, INTER_CUBIC);

	 /// Convert image to gray and blur it
	
	// cvtColor(src, src_gray, CV_BGR2GRAY);
	// blur(src_gray, src_gray, Size(3, 3));

	 

	//************ WINDOW **************
	namedWindow("CONTOUR", CV_WINDOW_AUTOSIZE);
	namedWindow("ORIGINAL", CV_WINDOW_AUTOSIZE);

	imgWidth = src.size().width;
	imgHeight = src.size().height;

	createTrackbar("Canny thresh:", "CONTOUR", &thresh, max_thresh, thresh_callback2);
	createTrackbar("Blur:", "CONTOUR", &blurValue, 8, thresh_callback);
	//createTrackbar("Corner thresh:", "CONTOUR", &thresh_corner, max_thresh, detectCorners);
	//createTrackbar("Bright:", "CONTOUR", &beta, 300, detectCorners);
	//createTrackbar("Contrast:", "CONTOUR", &alpha, 3, detectCorners);
	
	detectCorners(0, 0);





	cout << "width=" << src.size().width << "\n";

	while (1)
	{
		Sleep(2);

		thresh_callback(thresh, 0);
		
		cap >> src;
		cvtColor(src, src_gray, CV_BGR2GRAY);

		int key = waitKey(10);
		if (key == 27) break;
	}
	return 0;
}