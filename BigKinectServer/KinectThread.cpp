#include "KinectThread.h"
#include <gdiplus.h>
#pragma comment(lib,"gdiplus.lib")


KinectProvider *_kinect = nullptr;

void setKinectProvider(KinectProvider* KP) {
	_kinect = KP;
}

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

void KinectThread::connectClient(Client *C) {
	for (auto c : connectedClients) {
		if (c == C) return;
	}
	connectedClients.push_back(C);
}

void KinectThread::sendToClient() {
	char *c = 0;
	int length;
	run();
}

ImageThread::ImageThread() : KinectThread() {
}

ImageThread::~ImageThread() {}

void ImageThread::run() {
	void* img = collectImage(capacity);
	lck.lock();
	if (image) delete[] image;
	image = img;
	lck.unlock();
}

void ImageThread::getImage(void **image, UINT &cap) {
	cap = capacity;
	if (this->image == 0) {
		return;
	}
	lck.lock();
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

BYTE clip(int i) {
	if (i < 0) return 0;
	if (i > 255) return 255;
	return i;
}

int *YUV2RGB;

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
	OutputDebugString("Done");
}

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

BYTE* convertToJPEG(BYTE* RGBArray, UINT &length) {
	BITMAPINFO bmi;
	memset(&bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = 1920;
	bmi.bmiHeader.biHeight = -1080;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount = 24;

	// Write pixels to 'data' however you want...
	Gdiplus::Bitmap* myImage = new Gdiplus::Bitmap(&bmi, RGBArray);
	IStream *jpgStream;
	CLSID jpgClsid;
	GetEncoderClsid(L"image/jpeg", &jpgClsid);
	CreateStreamOnHGlobal(NULL, TRUE, &jpgStream);
	myImage->Save(jpgStream, &jpgClsid);
	STATSTG stats;
	jpgStream->Stat(&stats, STATFLAG_NONAME);
	BYTE *jpg = new BYTE[stats.cbSize.QuadPart];
	ULONG read;
	LARGE_INTEGER lg;
	lg.QuadPart = 0;
	jpgStream->Seek(lg, STREAM_SEEK_SET, NULL);
	jpgStream->Read(jpg, stats.cbSize.QuadPart, &read);
	jpgStream->Release();
	length = stats.cbSize.QuadPart;
	return jpg;

}

#include <ctime>
void *ColorImageThread::collectImage(UINT &cap) {
	unsigned start = clock();
	BYTE *image = nullptr;
	UINT capacity;
	while (_kinect->getImage(&image, capacity, true) != _kinect->OK);
	string s = to_string(clock() - start) + " , "s;
	OutputDebugString(s.c_str());

	start = clock();
	BYTE *imageRGB = new BYTE[capacity * 3 / 2];
	UINT iR = 0;
	for (int i = 0; i < capacity; i += 4, iR += 6) {
		convertToRGB(&image[i], &imageRGB[iR]);
	}
	capacity *= sizeof(BYTE) * 3 / 2;
	delete[] image;
	s = to_string(clock() - start) + " , "s;
	OutputDebugString(s.c_str());
	
	start = clock();
	BYTE *JPG = convertToJPEG(imageRGB, capacity);
	delete[] imageRGB;
	s = to_string(clock() - start) + "\n"s;
	OutputDebugString(s.c_str());
	cap = capacity;
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
	while (_kinect->getInfraredImage(&image, lCap, true) != _kinect->OK);
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
	while (_kinect->getDepthMap(&image, lCap, true) != _kinect->OK);
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
	while (_kinect->getBodyMap(&image, lCap, true) != _kinect->OK);
	lCap *= sizeof(BYTE);
	cap = lCap;
	return image;
}