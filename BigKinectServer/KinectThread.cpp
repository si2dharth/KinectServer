#include "KinectThread.h"
#include <gdiplus.h>
#pragma comment(lib,"gdiplus.lib")


KinectProvider *_kinect = nullptr;

void setKinectProvider(KinectProvider* KP) {
	_kinect = KP;
}

KinectProvider* getKinectProvider() {
	return _kinect;
}


KinectThread::KinectThread() : stopThread(false), backgroundWorker(&KinectThread::runThread, this) {
	if (!_kinect) throw "Kinect not set";
}

KinectThread::~KinectThread() {
	destroy();
};

void KinectThread::runThread() {
	while (!stopThread) {
		Sleep(3);							//Keeps CPU usage low and allows all threads to run
		this->run();
	}
}

void KinectThread::destroy() {
	stopThread = true;
	if (backgroundWorker.joinable())
		backgroundWorker.join();
}

ImageThread::ImageThread() : KinectThread(), lck() {
}

ImageThread::~ImageThread() {
	destroy();
	if (image) delete[] image;
}

void ImageThread::run() {
	UINT tmpCap;

	void* img = collectImage(tmpCap);
	lck.lock();								//As image is to be replaced, make sure no one else can access it
	capacity = tmpCap;
	if (image) delete[] image;				//Delete the old image
	image = img;
	lck.unlock();							//Replace done. Continue.
}

void ImageThread::getImage(void **image, UINT &cap) {
	if (this->image == 0) {
		return;
	}
	lck.lock();								//As the image is to be copied, make sure it is not deleted while copy is in progress.
	cap = capacity;
	*image = new BYTE[cap];
	memcpy(*image, this->image, cap);
	lck.unlock();
}


void ColorImageThread::initialize() {
	bool initialized = false;
	while (!initialized) {
		try {
			_kinect->startColorCapture();
			initialized = true;
		}
		catch (...) {}
	}
}

void ColorImageThread::finalize() {
	_kinect->stopColorCapture();
}

ColorImageThread::ColorImageThread() : ImageThread() {
}

ColorImageThread::~ColorImageThread() {
	destroy();
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
void *ColorImageThread::collectImage(UINT &capacity) {
	BYTE *image = nullptr;
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
	return JPG;
}

void InfraredImageThread::initialize() {
	bool initialized = false;
	while (!initialized) {
		try {
			_kinect->startInfraredCapture();
			initialized = true;
		} 
		catch (...) {}
	} 
}

void InfraredImageThread::finalize() {
	_kinect->stopInfraredCapture();
}

InfraredImageThread::InfraredImageThread() : ImageThread() {
}

InfraredImageThread::~InfraredImageThread() {
	destroy();
}

void *InfraredImageThread::collectImage(UINT &cap) {
	UINT16 *image = nullptr;
	while (_kinect->getInfraredImage(&image, cap, true) != _kinect->OK);		//Wait till image is successfully obtained
	cap *= sizeof(UINT16);
	return image;
}

void DepthMapThread::initialize() {
	bool initialized = false;
	while (!initialized) {
		try {
			_kinect->startDepthMapCapture();
			initialized = true;
		}
		catch (...) {}
	}
}

void DepthMapThread::finalize() {
	_kinect->stopDepthMapCapture();
}

DepthMapThread::DepthMapThread() : ImageThread() {
	
}

DepthMapThread::~DepthMapThread() {
	destroy();
}

void *DepthMapThread::collectImage(UINT &cap) {
	UINT16 *image = nullptr;
	while (_kinect->getDepthMap(&image, cap, true) != _kinect->OK);			//Wait till map is successfully obtained
	cap *= sizeof(UINT16);
	return image;
}


void BodyMapThread::initialize() {
	bool initialized = false;
	while (!initialized) {
		try {
			_kinect->startBodyMapCapture();
			initialized = true;
		} 
		catch (...) {}
	}
}

void BodyMapThread::finalize() {
	_kinect->stopBodyMapCapture();
}

BodyMapThread::BodyMapThread() : ImageThread() {
	//_kinect->startBodyMapCapture();
}

BodyMapThread::~BodyMapThread() {
	destroy();
	_kinect->stopBodyMapCapture();
}

void *BodyMapThread::collectImage(UINT &cap) {
	BYTE *image = nullptr;
	BYTE *compressedImage = nullptr;
	while (_kinect->getBodyMap(&image, cap, true) != _kinect->OK);				//Wait till map is successfully obtained.
	compressedImage = new BYTE[cap / 2];
	cap *= sizeof(BYTE);
	for (int i = 0; i < cap; i += 2) {
		compressedImage[i >> 1] = (image[i] << 4) + image[i + 1];
	}
	delete[]image;

	cap /= 2;
	return compressedImage;
}


BodyFrameProvider* BodyThread::bodyFP;

void BodyThread::initialize() {
	bodyFP = new BodyFrameProvider(_kinect);
}

void BodyThread::finalize() {
	delete bodyFP;
	bodyFP = nullptr;
}

BodyThread::BodyThread() : KinectThread() {};

BodyThread::~BodyThread() {
	destroy();
}

void BodyThread::run() {
	//lck.lock();
	bodyFP->updateFrame();
	//lck.unlock();
}

int BodyThread::getJoint(Joint *J, int bodyNumber, JointType jointType) {
	bodyFP->startDataCollection();
	if (bodyNumber > getNumberOfBodies()) {
		J->TrackingState = TrackingState_NotTracked;
		bodyFP->stopDataCollection();
		return -1;
	};
	set<int> bodies = bodyFP->getBodyIndices();
	auto body = bodies.begin();
	while (--bodyNumber > 0) {
		if (next(body) == bodies.end())
			break;
		else
			body = next(body);
	}

	if (body != bodies.end())
		*J = bodyFP->getJoint(*body, jointType);
	else
		J->TrackingState = TrackingState_NotTracked;
	bodyFP->stopDataCollection();
	return -1;
}

int BodyThread::getHandState(bool *closed, int bodyNumber, int side) {
	bodyFP->startDataCollection();
	set<int> bodies = bodyFP->getBodyIndices();
	auto body = bodies.begin();
	while (--bodyNumber > 0) {
		if (next(body) == bodies.end())
			break;
		else
			body = next(body);
	}

	if (body != bodies.end())
		if (side == 1)
			*closed = bodyFP->getLeftHandClosed(*body);
		else
			*closed = bodyFP->getRightHandClosed(*body);
	else
		*closed = false;
	bodyFP->stopDataCollection();
	return 0;
}

int BodyThread::getNumberOfBodies() {
	bodyFP->startDataCollection();
	int i = bodyFP->getNumberOfBodies();
	bodyFP->stopDataCollection();
	return i;
}

AudioThread::AudioThread() : KinectThread(), curUser(0) {}

AudioThread::~AudioThread() {}

int AudioThread::registerUser() {
	return curUser++;
}

void AudioThread::unregisterUser(int userID) {
	spokenWords.erase(userID);
	for (const string &s : phraseDict[userID]) {
		bool remove = true;
		for (auto &p : phraseDict) {
			if (p.first == userID) continue;
			if (p.second.find(s) != p.second.end()) {
				remove = false;
				break;
			}
		}
		if (remove) speechP->removeFromGrammar(s);
	}
	phraseDict.erase(userID);
}

void AudioThread::run() {
	edit.lock();
	string s = speechP->getSpokenWord();
	if (s != "") {
		q.lock();
		for (auto &p : spokenWords) {
			if (phraseDict[p.first].find(s) != phraseDict[p.first].end()) {
				p.second.push(s);
			}
		}
		q.unlock();
	}
	edit.unlock();
}

void AudioThread::addPhrase(int userID, string phrase) {
	edit.lock();
	speechP->addToGrammar(phrase);
	edit.unlock();
	phraseDict[userID].insert(phrase);
}

bool AudioThread::getSpokenPhrase(int userID, string &phrase) {
	bool result = false;
	q.lock();
	if (spokenWords[userID].size() > 0) {
		phrase = spokenWords[userID].front();
		result = true;
		spokenWords[userID].pop();
	}
	q.unlock();
	return result;
}

void AudioThread::initialize() {
	bool initialized = false;
	while (!initialized) {
		try {
			speechP = new SpeechProvider(getKinectProvider());
			initialized = true;
		} catch (...) {}
	}
}

void AudioThread::finalize() {
	delete speechP;
	speechP = nullptr;
}

SpeechProvider *AudioThread::speechP = nullptr;