#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <gl\gl.h>		
#include <gl\glu.h>	
#include <iostream>
#include <fstream>
#import <msxml6.dll> raw_interfaces_only


#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"

#include "ContourDetection.h"
#include "libxl.h"

using namespace cv;
using namespace std;
using namespace libxl;
//using namespace MSXML2;

//************* @XML
void saveXML()
{
	ofstream xml("config.xml");
	if (xml.is_open())
	{
		xml << "<?xml"; 
		xml << "version = ";
		xml << "'1.0' ?> \n";

		xml << "<config xmlns:dt='urn:schemas - microsoft - com : datatypes'>\n";
		xml << "<threshold dt:dt='int'>" << thresh << "</threshold>\n";
		xml << "<blur dt:dt='int'>" << blurValue << "</blur>\n";
		xml << "<bright dt:dt='int'>" << beta << "</bright>\n";
		xml << "<contrast dt:dt='int'>" << alpha << "</contrast>\n";
		xml << "<exposure dt:dt='int'>" << (exposure-10) << "</exposure>\n";
		xml << "<measure dt:dt='int'>" << calibTotal << "</measure>\n";
		xml << "</config>";

		xml.close();
	}
	cout << "CONFIG FILE SAVED!";
}
// Helper function to create a VT_BSTR variant from a null terminated string. 
HRESULT VariantFromString(PCWSTR wszValue, VARIANT &Variant)
{
	HRESULT hr = S_OK;
	BSTR bstr = SysAllocString(wszValue);
	CHK_ALLOC(bstr);

	V_VT(&Variant) = VT_BSTR;
	V_BSTR(&Variant) = bstr;

CleanUp:
	return hr;
}

// Helper function to create a DOM instance. 
HRESULT CreateAndInitDOM(IXMLDOMDocument **ppDoc)
{
	HRESULT hr = CoCreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(ppDoc));
	if (SUCCEEDED(hr))
	{
		// these methods should not fail so don't inspect result
		(*ppDoc)->put_async(VARIANT_FALSE);
		(*ppDoc)->put_validateOnParse(VARIANT_FALSE);
		(*ppDoc)->put_resolveExternals(VARIANT_FALSE);
	}
	return hr;
}
HRESULT ReportParseError(IXMLDOMDocument *pDoc, char *szDesc)
{
	HRESULT hr = S_OK;
	HRESULT hrRet = E_FAIL; // Default error code if failed to get from parse error.
	IXMLDOMParseError *pXMLErr = NULL;
	BSTR bstrReason = NULL;

	CHK_HR(pDoc->get_parseError(&pXMLErr));
	CHK_HR(pXMLErr->get_errorCode(&hrRet));
	CHK_HR(pXMLErr->get_reason(&bstrReason));
	printf("%s\n%S\n", szDesc, bstrReason);

CleanUp:
	SAFE_RELEASE(pXMLErr);
	SysFreeString(bstrReason);
	return hrRet;
}

void loadDOM()
{
	HRESULT hr = S_OK;
	IXMLDOMDocument *pXMLDom = NULL;
	IXMLDOMParseError *pXMLErr = NULL;
	IXMLDOMNodeList *pNodes = NULL;
	IXMLDOMNode *pNode = NULL;

	BSTR bstrXML = NULL;
	BSTR bstrErr = NULL;
	VARIANT_BOOL varStatus;
	VARIANT varFileName;
	VariantInit(&varFileName);

	CHK_HR(CreateAndInitDOM(&pXMLDom));

	// XML file name to load
	CHK_HR(VariantFromString(L"config.xml", varFileName));
	CHK_HR(pXMLDom->load(varFileName, &varStatus));
	if (varStatus == VARIANT_TRUE)
	{
		CHK_HR(pXMLDom->get_xml(&bstrXML));
		printf("XML DOM loaded from config.xml:\n%S\n", bstrXML);
	}
	else
	{
		// Failed to load xml, get last parsing error
		CHK_HR(pXMLDom->get_parseError(&pXMLErr));
		CHK_HR(pXMLErr->get_reason(&bstrErr));
		printf("Failed to load DOM from config.xml. %S\n", bstrErr);
	}

	// Query a single node.
	BSTR bstrQuery1 = NULL;
	BSTR bstrQuery2 = NULL;
	BSTR bstrNodeName = NULL;
	BSTR bstrNodeValue = NULL;

	bstrQuery1 = SysAllocString(L"//config[1]/*");
	CHK_ALLOC(bstrQuery1);
	CHK_HR(pXMLDom->selectNodes(bstrQuery1, &pNodes));
	if (pNodes)
	{
	/*	printf("Result from selectSingleNode:\n");
		CHK_HR(pNode->get_nodeName(&bstrNodeName));
		printf("Node, <%S>:\n", bstrNodeName);
		SysFreeString(bstrNodeName);
		*/
		long length;
		CHK_HR(pNodes->get_length(&length));
		for (long i = 0; i < length; i++)
		{
			CHK_HR(pNodes->get_item(i, &pNode));
			CHK_HR(pNode->get_nodeName(&bstrNodeName));

			BSTR s;
			char *sc;
			int v;
			if (!wcscmp(bstrNodeName, L"bright"))
			{
				
				pNode->get_text(&s);
				sc = _com_util::ConvertBSTRToString(s);
				v = atoi(sc);
				beta = v;
			}
			else if (!wcscmp(bstrNodeName, L"contrast"))
			{
				pNode->get_text(&s);
				sc = _com_util::ConvertBSTRToString(s);
				v = atoi(sc);
				beta = v;
			}
			else if (!wcscmp(bstrNodeName, L"threshold"))
			{
				pNode->get_text(&s);
				sc = _com_util::ConvertBSTRToString(s);
				v = atoi(sc);
				thresh = v;
			}
			else if (!wcscmp(bstrNodeName, L"blur"))
			{
				pNode->get_text(&s);
				sc = _com_util::ConvertBSTRToString(s);
				v = atoi(sc);
				blurValue = v;
			}
			else if (!wcscmp(bstrNodeName, L"exposure"))
			{
				pNode->get_text(&s);
				sc = _com_util::ConvertBSTRToString(s);
				v = atoi(sc);
				exposure = v;
			}
			else if (!wcscmp(bstrNodeName, L"measure"))
			{
				pNode->get_text(&s);
				sc = _com_util::ConvertBSTRToString(s);
				v = atoi(sc);
				calibTotal = v;
			}

			SysFreeString(bstrNodeName);
			CHK_HR(pNode->get_xml(&bstrNodeValue));
			SysFreeString(bstrNodeValue);
			SAFE_RELEASE(pNode);
		}
	}
	else
	{
		CHK_HR(ReportParseError(pXMLDom, "Error while calling selectSingleNode."));
	}


CleanUp:
	SAFE_RELEASE(pXMLDom);
	SAFE_RELEASE(pXMLErr);
	SysFreeString(bstrXML);
	SysFreeString(bstrErr);
	VariantClear(&varFileName);
}

void drawStatus(Mat clone, Scalar color, char* text)
{
	rectangle(clone, Rect(0.0, 0.0, imgWidth, 20), color, CV_FILLED);

	putText(clone, text, Point((imgWidth / 2) - 15, 13), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(255, 255, 255), 1);
}
void calibrate()
{
	isCalib = 1;
}
void calibrating(Mat canny_output)
{
	int lInc = 0;
	int rInc = 0;

	for (int j = canny_output.cols / 2; j > 0; j--)
	{
		lInc++;
		if ((int)canny_output.at<uchar>(canny_output.rows / 2, j) > 0)
		{
			break;
		}
	}
	for (int k = canny_output.cols / 2; k < canny_output.cols; k++)
	{
		rInc++;
		if ((int)canny_output.at<uchar>(canny_output.rows / 2, k) > 0)
		{
			break;
		}
	}

	calibTotal = lInc + rInc;

	cout << "CALIB: " << calibTotal;

	isCalib = 0;
}
int getMeasure(int value)
{
	if (calibTotal == 0)
		return(0);
	int x = (210 * value) / calibTotal;
	return(x);
}
void writeXLS(int x, int y, float rot)
{
	Book* book = xlCreateBook(); // xlCreateXMLBook() for xlsx
	if (book)
	{
		Sheet* sheet = book->addSheet("Sheet1");
		if (sheet)
		{
			sheet->writeStr(1, 0, "WRITE");
			sheet->writeNum(1, 1, 0);
			sheet->writeStr(2, 0, "X");
			sheet->writeNum(2, 1, x);
			sheet->writeStr(3, 0, "Y");
			sheet->writeNum(3, 1, y);
			sheet->writeStr(4, 0, "ROT");
			sheet->writeNum(4, 1, rot);
			sheet->writeStr(5, 0, "FAULT");
			sheet->writeNum(5, 1, 0);
		}
		book->save("coords.xls");
		book->release();
	}
}
void readXLS()
{
	Book* book = xlCreateBook();

	if (book->load("coords.xls"))
	{
		Sheet* sheet = book->getSheet(0);
		if (sheet)
		{
			cout << "READ: " << sheet->readNum(1, 1) << "\n" << "X: " << sheet->readNum(2, 1) << "\n" << "y: " << sheet->readNum(3, 1) << "\n" << "ROT: " << sheet->readNum(4, 1) << "\n\n";
		}
	}
	book->release();
}
int getReadTag()
{
	Book* book = xlCreateBook();
	int read = 0;

	if (book->load("coords.xls"))
	{
		Sheet* sheet = book->getSheet(0);
		if (sheet)
		{
			read = sheet->readNum(1, 1);
		}
	}
	book->release();

	Book* xml = xlCreateXMLBook();
	if (xml->load("config.xml"))
	{
		
	}

	return(read);
}
void setBright(int, void*)
{
	cap.set(CV_CAP_PROP_BRIGHTNESS, beta);
}
void setContrast(int, void*)
{
	cap.set(CV_CAP_PROP_CONTRAST, alpha);
}
void setExposure(int, void*)
{
	cap.set(CV_CAP_PROP_EXPOSURE, exposure-10);
}
void setOffset(int, void*)
{
	cap.set(CV_CAP_PROP_XI_OFFSET_X, offsetX - 100);
}

void detectCorners(int, void*)
{
	Mat dst, dst_norm, dst_norm_scaled;
	dst = Mat::zeros(src.size(), CV_32FC1);
	Mat clone = src.clone();
	Mat canny_output;
	Canny(src_gray, canny_output, thresh, thresh * 4, 3);

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
	Mat canny_output, gray_clone;
	Mat blured;
	Mat lap;
	if (blurValue != 0)
		blur(src_gray, blured, Size(blurValue, blurValue));
	else
		blured = src_gray;
	//Laplacian(blured, lap,CV_8S);

	blured.convertTo(lap, -1, alpha, beta);

	gray_clone = blured.clone();
	//threshold(gray_clone, gray_clone, 220, 255, THRESH_BINARY);

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
	int col2L = 0;
	int col2R = 0;
	const int INC_HEIGHT = 60;

		//***************** CALIBRATION
	if (isCalib == 1)
		calibrating(canny_output);
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
		//**** 2ND LATERAIS
		for (int j = canny_output.cols / 2; j > 0; j--)
		{
			if ((int)canny_output.at<uchar>((canny_output.rows / 2)+INC_HEIGHT, j) > 0)
			{
				col2L = j;
				break;
			}
		}
		for (int k = canny_output.cols / 2; k < canny_output.cols; k++)
		{
			if ((int)canny_output.at<uchar>((canny_output.rows / 2)+INC_HEIGHT, k) > 0)
			{
				col2R = k;
				break;
			}
		}
		int wR = canny_output.cols - colR;
		int wL = colL;
		int dif = (wR - wL) / 2;

		line(clone, Point(colL, imgHeight / 2 - 10), Point(colL, imgHeight / 2 + 10), Scalar(0, 0, 255), 1, CV_AA);
		line(clone, Point(colR, imgHeight / 2 - 10), Point(colR, imgHeight / 2 + 10), Scalar(0, 0, 255), 1, CV_AA);

		line(clone, Point(col2L, (imgHeight / 2)+INC_HEIGHT - 10), Point(col2L, (imgHeight / 2)+INC_HEIGHT + 10), Scalar(0, 0, 255), 1, CV_AA);
		line(clone, Point(col2R, (imgHeight / 2)+INC_HEIGHT - 10), Point(col2R, (imgHeight / 2)+INC_HEIGHT + 10), Scalar(0, 0, 255), 1, CV_AA);
		//***************** @@LATERAIS
		//***************** GOLA
		for (int i = canny_output.rows / 2; i > 0; i--)
		{
			if ((int)canny_output.at<uchar>(i, (canny_output.cols / 2) - dif) > 0 && rowM == 0)
			{
				rowM = i;
			}
			if ((int)canny_output.at<uchar>(i, (canny_output.cols / 2 - 20) - dif) > 0 && rowL == 0)
			{
				rowL = i;
			}
			if ((int)canny_output.at<uchar>(i, (canny_output.cols / 2 + 20) - dif) > 0 && rowR == 0)
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

		deg = ang*(180 / 3.14159265);

		//********* RECTA MEDIANA
		int media1 = (colR - colL) / 2 + colL;
		int media2 = (col2R - col2L) / 2 + col2L;
		line(clone, Point(media1, (imgHeight / 2)), Point(media2, (imgHeight / 2) + INC_HEIGHT), Scalar(0, 255, 255), 1, CV_AA);
		double tm = ((imgHeight / 2) + INC_HEIGHT) - (imgHeight / 2);
		double bm = media2 - media1;

		double mm = 0;
		if(bm!=0)
			mm = tm / bm;
		

		double angM = atan(mm);
		double degM = angM*(180 / 3.14159265);
		degM = degM - 90;
	//	cout << "ang: " << angM << "\n";
	//	cout << "deg: " << degM << "\n";

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
		int frst = 0;

		for (int m = 0; m < sizeof(rows); m++)
		{
			if (rows[m]>frst)
				frst = rows[m];
		}
		difH = -frst;

		Point c1 = Point(imgWidth / 2 - 20 - dif, imgHeight / 2 - difH);
		Point c2 = Point(imgWidth / 2 + 20 - dif, imgHeight / 2 - difH);
		Point c3 = Point(imgWidth / 2 - dif, imgHeight / 2 - 20 - difH);
		Point c4 = Point(imgWidth / 2 - dif, imgHeight / 2 + 20 - difH);
		line(clone, c1, c2, Scalar(100, 0, 255), 1, CV_AA);
		line(clone, c3, c4, Scalar(100, 0, 255), 1, CV_AA);

		int gripX = ((imgWidth / 2) - dif);
		int gripY = ((imgHeight / 2) - difH);
		if (degM == 90 || degM == -90)
			degM = 0;
		drawGripper(clone, Point2f(gripX, gripY), degM);

		

		if (getReadTag() == 1)
		{
			float tmp = degM * 100;
			int tmp2 = tmp;
			double tmp3 = (double)tmp2 / 100.00;
			if (degM == 90 || degM == -90)
				degM = 0;
			if (degM < 10 && degM > -10 && getMeasure(dif) < 100 && getMeasure(difH) < 100)
			{
				drawStatus(clone, Scalar(0, 255, 0), "WRITE");
				Sleep(10);
				writeXLS(getMeasure(dif), getMeasure(difH), tmp3);
			}
		}
		else
			drawStatus(clone, Scalar(0, 0, 255), "WAIT");

	imshow("CONTOUR", canny_output);
	imshow("ORIGINAL", clone);
	imshow("GRAY", gray_clone);
}

void getCameraInfo(VideoCapture m_cam){
	std::cout << "CV_CAP_PROP_FRAME_WIDTH " << m_cam.get(CV_CAP_PROP_FRAME_WIDTH) << std::endl;
	std::cout << "CV_CAP_PROP_FRAME_HEIGHT " << m_cam.get(CV_CAP_PROP_FRAME_HEIGHT) << std::endl;
	std::cout << "CV_CAP_PROP_FPS " << m_cam.get(CV_CAP_PROP_FPS) << std::endl;
	std::cout << "CV_CAP_PROP_EXPOSURE " << m_cam.get(CV_CAP_PROP_EXPOSURE) << std::endl;
	std::cout << "CV_CAP_PROP_FORMAT " << m_cam.get(CV_CAP_PROP_FORMAT) << std::endl; //deafult CV_8UC3?!
	std::cout << "CV_CAP_PROP_CONTRAST " << m_cam.get(CV_CAP_PROP_CONTRAST) << std::endl;
	std::cout << "CV_CAP_PROP_BRIGHTNESS " << m_cam.get(CV_CAP_PROP_BRIGHTNESS) << std::endl;
	std::cout << "CV_CAP_PROP_SATURATION " << m_cam.get(CV_CAP_PROP_SATURATION) << std::endl;
	std::cout << "CV_CAP_PROP_HUE " << m_cam.get(CV_CAP_PROP_HUE) << std::endl;
	std::cout << "CV_CAP_PROP_POS_FRAMES " << m_cam.get(CV_CAP_PROP_POS_FRAMES) << std::endl;
	std::cout << "CV_CAP_PROP_FOURCC " << m_cam.get(CV_CAP_PROP_FOURCC) << std::endl;

	int ex = static_cast<int>(m_cam.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form
	char EXT[] = { (char)(ex & 255), (char)((ex & 0XFF00) >> 8), (char)((ex & 0XFF0000) >> 16), (char)((ex & 0XFF000000) >> 24), 0 };
	cout << "Input codec type: " << EXT << endl;
}
int _tmain(int argc, _TCHAR* argv[])
{
	printf("==============================================================================\n");
	printf("== CONTOUR DETECTION                                TARAMBOLA DEVELOPMENT ==\n");
	printf("==============================================================================\n\n");

	//**** load config xml
	HRESULT hr = CoInitialize(NULL);
	if (SUCCEEDED(hr))
	{
		loadDOM();
		CoUninitialize();
	}

	//************** CAMERA
	cap.open(1);
	//cap.set(CV_CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G'));
	cap.set(CV_CAP_PROP_SHARPNESS, 20);
	cap.set(CV_CAP_PROP_FPS, 12);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
	//cap.open("video.MP4");
	if (!cap.isOpened())
	{
		cout << "CAMERA NOT CONNECTED!";
		return(-1);
	}
	getCameraInfo(cap);
	
	//»»»»»»»»»»»»»» IMAGE
	// src_gray = imread( "red2.jpg", 0);
	// src = imread("red2.jpg");

	Sleep(2);
	//cap.set(CV_CAP_PROP_MODE, -2);
	Sleep(2);
	//cap.set(CV_CAP_PROP_FPS, 1);
	Sleep(2);
//	cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	Sleep(2);
//	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	Sleep(2);
	cap.set(CV_CAP_PROP_BRIGHTNESS, beta);
	Sleep(2);
	cap.set(CV_CAP_PROP_CONTRAST, alpha);
	Sleep(2);
	cap.set(CV_CAP_PROP_EXPOSURE, exposure);
	Sleep(2);
	cap.set(CV_CAP_PROP_ZOOM, 0);
	//Sleep(2);
	//cap.set(CV_CAP_PROP_XI_OFFSET_X, 100);
	
	cout << cap.get(CV_CAP_PROP_EXPOSURE);
	cout << "\n" << cap.get(CV_CAP_PROP_FRAME_WIDTH);

	while (cap.get(CV_CAP_PROP_FRAME_WIDTH) == 0)
	{
		cout << "\n" << cap.get(CV_CAP_PROP_FRAME_WIDTH);
		Sleep(1000);
	}

	cap >> src;

	cap.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
	
	if (src.empty())
		return(-2);
	else if (src.channels()>1)
		cvtColor(src, src_gray, CV_BGR2GRAY);
	else src_gray = src;
	
	//cvtColor(src, src_gray, CV_BGR2GRAY);

	


	// resize(src, src, Size(), 0.5, 0.5, INTER_CUBIC);
	// resize(src_gray, src_gray, Size(), 0.5, 0.5, INTER_CUBIC);

	 /// Convert image to gray and blur it
	
	// cvtColor(src, src_gray, CV_BGR2GRAY);
	// blur(src_gray, src_gray, Size(3, 3));

	 

	//************ WINDOW **************
	namedWindow("CONTOUR", CV_WINDOW_AUTOSIZE);
	namedWindow("ORIGINAL", CV_WINDOW_AUTOSIZE);
	namedWindow("GRAY", CV_WINDOW_AUTOSIZE);

	imgWidth = src.size().width;
	imgHeight = src.size().height;

	createTrackbar("Canny thresh:", "CONTOUR", &thresh, max_thresh, thresh_callback2);
	createTrackbar("Blur:", "CONTOUR", &blurValue, 8, thresh_callback);
	//createTrackbar("Corner thresh:", "CONTOUR", &thresh_corner, max_thresh, detectCorners);
	createTrackbar("Bright:", "CONTOUR", &beta, 300, setBright);
	createTrackbar("Contrast:", "CONTOUR", &alpha, 20, setContrast);
	createTrackbar("Exposure:", "CONTOUR", &exposure, 20, setExposure);
	//createTrackbar("OffsetX:", "CONTOUR", &offsetX, 200, setOffset);
	
	detectCorners(0, 0);





	cout << "width=" << src.size().width << "\n";

	while (1)
	{
		Sleep(100);
	//	resize(src, src, Size(), 0.1, 0.1, INTER_CUBIC);
	//	resize(src_gray, src_gray, Size(), 0.1, 0.1, INTER_CUBIC);


		cap.read(src);

		//cap >> src;
		if (src.empty())
			return(-2);
		else if (src.channels()>1)
			cvtColor(src, src_gray, CV_BGR2GRAY);
		else src_gray = src;
		//cvtColor(src, src_gray, CV_BGR2GRAY);

		thresh_callback(thresh, 0);


		int key = waitKey(10);
		if (key == 99)
			calibrate();
		if (key == 115)
			saveXML();
		if (key == 27) break;
	}
	return 0;
}