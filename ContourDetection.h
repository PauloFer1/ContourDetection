using namespace std;
using namespace cv;

#define PI 3.14159265;
// Macro that calls a COM method returning HRESULT value.
#define CHK_HR(stmt)        do { hr=(stmt); if (FAILED(hr)) goto CleanUp; } while(0)
// Macro to verify memory allcation.
#define CHK_ALLOC(p)        do { if (!(p)) { hr = E_OUTOFMEMORY; goto CleanUp; } } while(0)
// Macro that releases a COM object if not NULL.
#define SAFE_RELEASE(p)     do { if ((p)) { (p)->Release(); (p) = NULL; } } while(0)

Mat src, src_gray, src_canny;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);
float nearD = 1000;
Point near1, near2;
int imgWidth;
int imgHeight;
int thresh_corner = 200;
int alpha = 10; /**< Simple contrast control */
int beta = 0;  /**< Simple brightness control */
int exposure = 6;
int blurValue = 1;
VideoCapture cap;
int isCalib = 0;
int calibTotal = 0;
int offsetX = 100;

//** XML
void saveXML();
void loadDOM();
HRESULT VariantFromString(PCWSTR wszValue, VARIANT &Variant);
HRESULT CreateAndInitDOM(IXMLDOMDocument **ppDoc);
HRESULT ReportParseError(IXMLDOMDocument *pDoc, char *szDesc);

//** XLS
void writeXLS(int x, int y, float rot);
void readXLS();

//** SET CAMERA CONFIG
void setBright(int, void*);
void setContrast(int, void*);
void setExposure(int, void*);
void setOffset(int, void*);
void thresh_callback(int, void*);

void getCameraInfo(VideoCapture m_cam);

void drawStatus(Mat clone, Scalar color, char* text);
int getMeasure(int value);
int getReadTag();

//** VIDEO PROCESSING
void calibrating(Mat canny_output);
void calibrate();
void detectCorners(int, void*);
void thresh_callback2(int, void*);
void drawGripper(Mat img, Point2f center, float angle);

int _tmain(int argc, _TCHAR* argv[]);