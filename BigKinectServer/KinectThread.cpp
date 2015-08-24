#include "KinectThread.h"
#include <gdiplus.h>
#pragma comment(lib,"gdiplus.lib")


KinectProvider *_kinect = nullptr;

void setKinectProvider(KinectProvider* KP) {
	_kinect = KP;
}

///A new thread procedure that simply calls the run method for a given KinectThread
DWORD __stdcall startThread(void *kinectThread) {
	KinectThread *kThread = (KinectThread*)kinectThread;
	while (true) {
		kThread->run();
	}
}

KinectThread::KinectThread() {
	if (!_kinect) throw "Kinect not set";
}

KinectThread::~KinectThread() {};

ImageThread::ImageThread() : KinectThread(),lck() {
}

ImageThread::~ImageThread() {
	if (image) delete[] image;
}

void ImageThread::run() {
	void* img = collectImage(capacity);		//WARNING: capacity is changed here and its value is NOT locked.
	lck.lock();								//As image is to be replaced, make sure no one else can access it
	if (image) delete[] image;				//Delete the old image
	image = img;							
	lck.unlock();							//Replace done. Continue.
}

void ImageThread::getImage(void **image, UINT &cap) {
	cap = capacity;
	if (this->image == 0) {
		return;
	}
	lck.lock();								//As the image is to be copied, make sure it is not deleted while copy is in progress.
	*image = new BYTE[cap];
	memcpy(*image, this->image, cap);
	lck.unlock();
}


ColorImageThread::ColorImageThread() : ImageThread() {
	_kinect->startColorCapture();
}

ColorImageThread::~ColorImageThread() {
	_kinect->stopColorCapture();
}

///Make sure the given value lies between 0 and 255
BYTE clip(int i) {
	if (i < 0) return 0;
	if (i > 255) return 255;
	return i;
}

///Array of YUV to RGB conversion. Too big, so initialize dynamically.
int *YUV2RGB;

/**
To save in one big array, the following method is used:
Y,U,V are each 8 bits.
Form a 24 bit integer with YUV and save the corresponding RGB integer with a 24 bits.
*/
void calConv() {
	YUV2RGB = new int[256 * 256 * 256];
	for (int Y = 0; Y < 255; Y++) {
		for (int U = 0; U < 255; U++) {
			for (int V = 0; V < 255; V++) {
				int C, D, E;
				C = Y - 16;
				D = U - 128;
				E = V - 128;
				BYTE R, G, B;
				B = clip((C * 298 + 409 * E + 128) >> 8);
				G = clip((C * 298 - 100 * D - 208 * E + 128) >> 8);
				R = clip((C * 298 + 516 * D + 128) >> 8);
				YUV2RGB[(Y << 16) + (U << 8) + V] = (B << 16) + (G << 8) + R;
			}
		}
	}
}

///Convert a given YUV2 coordinate to RGB
/**
In a YUV2 image, 4 bytes are used as Y1 U Y2 V. 4 bytes describe 2 pixels: Y1 U V describes first one, while Y2 U V describes second one.
In a RGB image, 6 bytes are needed for 2 pixels.

Conversion is easy. simply use the array.
*/
inline void convertToRGB(BYTE* YUV, BYTE* RGB) {
	BYTE Y = YUV[0], U = YUV[1], V = YUV[3];
	int yuv = (Y << 16) + (U << 8) + (V);
	int rgb = YUV2RGB[yuv];
	RGB[2] = rgb >> 16;
	RGB[1] = (rgb & 0xFF00) >> 8;
	RGB[0] = rgb & 0xFF;

	Y = YUV[2];
	yuv = (Y << 16) + (U << 8) + V;
	rgb = YUV2RGB[yuv];

	RGB[5] = rgb >> 16;
	RGB[4] = (rgb & 0xFF00) >> 8;
	RGB[3] = rgb & 0xFF;
}

///Get encoder method for converting BMP to JPG. Method from https://msdn.microsoft.com/en-us/library/windows/desktop/ms533843(v=vs.85).aspx
using namespace Gdiplus;
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

///Convert the given RGB array to a JPEG image. The JPEG image is returned in a BYTE array while the length of the array is returned through parameter
BYTE* convertToJPEG(BYTE* RGBArray, UINT &length) {
	BITMAPINFO bmi;											//Create bitmap header
	memset(&bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = 1920;
	bmi.bmiHeader.biHeight = -1080;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount = 24;

	Gdiplus::Bitmap* myImage = new Gdiplus::Bitmap(&bmi, RGBArray);		//Form bitmap out of provided RGB array
	IStream *jpgStream;
	CLSID jpgClsid;
	GetEncoderClsid(L"image/jpeg", &jpgClsid);				//Get the encoder
	CreateStreamOnHGlobal(NULL, TRUE, &jpgStream);			//Get direct access to physical memory. Create a stream to save directly into it. Delete when stream is released
	myImage->Save(jpgStream, &jpgClsid);					//Save the jpg image into physical memory
	STATSTG stats;
	jpgStream->Stat(&stats, STATFLAG_NONAME);				//Get stats of the jpg image; more importantly, the size
	BYTE *jpg = new BYTE[stats.cbSize.QuadPart];			//Create byte array for transferring image to.
	ULONG read;
	LARGE_INTEGER lg;
	lg.QuadPart = 0;
	jpgStream->Seek(lg, STREAM_SEEK_SET, NULL);				//Move to beginning of stream
	jpgStream->Read(jpg, stats.cbSize.QuadPart, &read);		//Read entire stream into the array
	jpgStream->Release();									//Release the stream
	length = stats.cbSize.QuadPart;							//Save the length of the byte array
	return jpg;

}

#include <ctime>
void *ColorImageThread::collectImage(UINT &cap) {
	BYTE *image = nullptr;
	UINT capacity;															//Use a copy capacity variable so that the variable value is not disturbed in the calling function (It is a reference variable)
	while (_kinect->getImage(&image, capacity, true) != _kinect->OK);		//Wait until a new image is received
	BYTE *imageRGB = new BYTE[capacity * 3 / 2];							//RGB is 6/4 = 3/2 times bigger than YUV2 image
	UINT iR = 0;
	for (int i = 0; i < capacity; i += 4, iR += 6) {
		convertToRGB(&image[i], &imageRGB[iR]);								//Convert the image to RGB 2 pixels at a time
	}
	capacity *= sizeof(BYTE) * 3 / 2;										//Update the capacity
	delete[] image;															//Delete the image received from the Kinect. Now, only imageRGB is required
	
	BYTE *JPG = convertToJPEG(imageRGB, capacity);							//Convert the RGB image to JPEG
	delete[] imageRGB;														//Now, imageRGB is also not needed. Only JPG is needed.
	cap = capacity;															//Update the capacity now.
	return JPG;
}


InfraredImageThread::InfraredImageThread() : ImageThread() {
	_kinect->startInfraredCapture();
}

InfraredImageThread::~InfraredImageThread() {
	_kinect->stopInfraredCapture();
}

void *InfraredImageThread::collectImage(UINT &cap) {
	UINT16 *image = nullptr;
	UINT lCap;
	while (_kinect->getInfraredImage(&image, lCap, true) != _kinect->OK);		//Wait till image is successfully obtained
	lCap *= sizeof(UINT16);
	cap = lCap;
	return image;
}

DepthMapThread::DepthMapThread() : ImageThread() {
	_kinect->startDepthMapCapture();
}

DepthMapThread::~DepthMapThread() {
	_kinect->stopDepthMapCapture();
}

void *DepthMapThread::collectImage(UINT &cap) {
	UINT16 *image = nullptr;
	UINT lCap;
	while (_kinect->getDepthMap(&image, lCap, true) != _kinect->OK);			//Wait till map is successfully obtained
	lCap *= sizeof(UINT16);
	cap = lCap;
	return image;
}

BodyMapThread::BodyMapThread() : ImageThread() {
	_kinect->startBodyMapCapture();
}

BodyMapThread::~BodyMapThread() {
	_kinect->stopBodyMapCapture();
}

void *BodyMapThread::collectImage(UINT &cap) {
	BYTE *image = nullptr;
	UINT lCap;
	while (_kinect->getBodyMap(&image, lCap, true) != _kinect->OK);				//Wait till map is successfully obtained.
	lCap *= sizeof(BYTE);
	cap = lCap;
	return image;
}