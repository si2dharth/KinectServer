#include "KinectProvider.h"
#include <set>
using namespace std;

KinectProvider::KinectProvider() :		//Initialize all to nullptr
	sensor(nullptr),
	audioBeamReader(nullptr),
	colorFrameReader(nullptr),
	bodyFrameReader(nullptr),
	bodyMapReader(nullptr),
	depthFrameReader(nullptr),
	infraredFrameReader(nullptr)
{
	int hr = GetDefaultKinectSensor(&sensor);	//Get the default caonnected sensor in case of more than one sensor
	if (hr != 0) {								//Failed, meaning sensor was not found
		sensor = nullptr;
		throw error::SensorNotFound;
	}
	hr = sensor->Open();
	if (hr != 0) {								//Failed, meaning sensor couldn't be connected.
		sensor->Release();
		throw error::SensorNotReady;
	}
}

KinectProvider::~KinectProvider() {
	//Release everything
	if (audioBeamReader) audioBeamReader->Release();
	if (colorFrameReader) colorFrameReader->Release();
	if (bodyFrameReader) bodyFrameReader->Release();
	if (bodyMapReader) bodyMapReader->Release();
	if (depthFrameReader) depthFrameReader->Release();
	if (infraredFrameReader) infraredFrameReader->Release();
	if (sensor) sensor->Release();
}


void KinectProvider::startColorCapture() {
	///If reader is open, don't do anything
	if (!colorFrameReader) {
														//Standard method: Get the source, get reader from source
		IColorFrameSource *ICS = nullptr;
		int hr = sensor->get_ColorFrameSource(&ICS);
		if (hr != 0) throw error::ColorSourceNotReady;
		hr = ICS->OpenReader(&colorFrameReader);
		ICS->Release();									//Don't need the source anymore
		if (hr != 0) {
			colorFrameReader = nullptr;					//Make sure its not set to an arbitrary value in case of failure
			throw error::CouldNotOpenColorReader;
		}
	}
	imageUsers++;										//Count the number of users for the reader.
}

void KinectProvider::stopColorCapture() {
	if (colorFrameReader) {
		if (--imageUsers > 0) return;
		colorFrameReader->Release();
		colorFrameReader = nullptr;
	}
}

void KinectProvider::startInfraredCapture() {							
	if (!infraredFrameReader) {
		//Standard method: Get the source, get reader from source		
		IInfraredFrameSource *IFS = nullptr;
		int hr = sensor->get_InfraredFrameSource(&IFS);
		if (hr != 0) throw error::InfraredSourceNotReady;
		hr = IFS->OpenReader(&infraredFrameReader);
		IFS->Release();										//Don't need the source anymore
		if (hr != 0) {
			infraredFrameReader = nullptr;					//Make sure its not set to an arbitrary value in case of failure
			throw error::CouldNotOpenInfraredReader;
		}
	}
	infraredUsers++;										//Count the number of users for the reader.
}

void KinectProvider::stopInfraredCapture() {
	if (!infraredFrameReader) return;
	if (--infraredUsers > 0) return;
	infraredFrameReader->Release();
	infraredFrameReader = nullptr;
}

void KinectProvider::startBodyDataCapture() {
	if (!bodyFrameReader) {
		//Standard method: Get the source, get reader from source
		IBodyFrameSource *IBS = nullptr;
		int hr = sensor->get_BodyFrameSource(&IBS);
		if (hr != 0) throw error::BodyDataSourceNotReady;
		hr = IBS->OpenReader(&bodyFrameReader);
		IBS->Release();										   //Don't need the source anymore
		if (hr != 0) {
			bodyFrameReader = nullptr;						   //Make sure its not set to an arbitrary value in case of failure
			throw error::CouldNotOpenBodyDataReader;
		}
	}
	bodyUsers++;											   //Count the number of users for the reader.
}

void KinectProvider::stopBodyDataCapture() {
	if (!bodyFrameReader) return;
	if (--bodyUsers > 0) return;
	bodyFrameReader->Release();
	bodyFrameReader = nullptr;
}

void KinectProvider::startDepthMapCapture() {
	if (!depthFrameReader) {
		//Standard method: Get the source, get reader from source
		IDepthFrameSource *IDS = nullptr;
		int hr = sensor->get_DepthFrameSource(&IDS);
		if (hr != 0) throw error::DepthMapSourceNotReady;
		hr = IDS->OpenReader(&depthFrameReader);
		IDS->Release();										 //Don't need the source anymore
		if (hr != 0) {
			depthFrameReader = nullptr;						 //Make sure its not set to an arbitrary value in case of failure
			throw error::CouldNotOpenDepthMapReader;
		}
	}
	depthUsers++;											 //Count the number of users for the reader.
}

void KinectProvider::stopDepthMapCapture() {
	if (!depthFrameReader) return;
	if (--depthUsers) return;
	depthFrameReader->Release();
	depthFrameReader = nullptr;
}

void KinectProvider::startBodyMapCapture() {
	if (!bodyMapReader) {
		//Standard method: Get the source, get reader from source
		IBodyIndexFrameSource *IBIS = nullptr;
		int hr = sensor->get_BodyIndexFrameSource(&IBIS);
		if (hr != 0) throw error::BodyMapSourceNotReady;
		hr = IBIS->OpenReader(&bodyMapReader);
		IBIS->Release();									//Don't need the source anymore
		if (hr != 0) {
			bodyMapReader = nullptr;						//Make sure its not set to an arbitrary value in case of failure
			throw error::CouldNotOpenBodyMapReader;
		}
	}
	bodyMapUsers++;											//Count the number of users for the reader.
}

void KinectProvider::stopBodyMapCapture() {
	if (!bodyMapReader) return;
	if (--bodyMapUsers) return;
	bodyMapReader->Release();
	bodyMapReader = nullptr;
}

void KinectProvider::startAudioCapture() {
	if (!audioBeamReader) {
		//Standard method: Get the source, get reader from source
		IAudioSource *IAS = nullptr;
		int hr = sensor->get_AudioSource(&IAS);
		if (hr != 0) throw error::AudioSourceNotReady;
		hr = IAS->OpenReader(&audioBeamReader);
		IAS->Release();										   //Don't need the source anymore
		if (hr != 0) {
			audioBeamReader = nullptr;						   //Make sure its not set to an arbitrary value in case of failure
			throw error::CouldNotOpenAudioReader;
		}
	}
	audioUsers++;											   //Count the number of users for the reader.
}

void KinectProvider::stopAudioCapture() {
	if (!audioBeamReader) return;
	if (--audioUsers) return;
	audioBeamReader->Release();
	audioBeamReader = nullptr;
}



int KinectProvider::getImage(OUT BYTE **image, OUT UINT &arraySize, bool copy) {
	if (!colorFrameReader) throw error::ColorCaptureNotStarted;			//Make sure that capture was started
	IColorFrame *ICF = nullptr;
	int hr = colorFrameReader->AcquireLatestFrame(&ICF);
	if (hr != 0) return result::NotReady;
	ICF->AccessRawUnderlyingBuffer(&arraySize, image);					//Access buffer regardless of copy parameter as arraySize is required
	if (copy) {
		*image = new BYTE[arraySize];									//Kinect does not manage array
		ICF->CopyRawFrameDataToArray(arraySize, *image);				
	}
	ICF->Release();
	return result::OK;
}

int KinectProvider::getInfraredImage(OUT UINT16 **image, OUT UINT &arraySize, bool copy) {
	if (!infraredFrameReader) throw error::InfraredCaptureNotStarted;		//Make sure that capture was started
	IInfraredFrame *IIF = nullptr;
	int hr = infraredFrameReader->AcquireLatestFrame(&IIF);
	if (hr != 0) return result::NotReady;
	IIF->AccessUnderlyingBuffer(&arraySize, image);							//Access buffer regardless of copy parameter as arraySize is required
	if (copy) {
		*image = new UINT16[arraySize];										//Kinect does not manage array
		IIF->CopyFrameDataToArray(arraySize, *image);
	}
	IIF->Release();
	return result::OK;
}


int KinectProvider::getBodyCount(OUT int &bodyCount) {
	UINT cap = 0;
	BYTE *buf;
	int res = getBodyMap(&buf, cap, true);									//Try to get the body index map
	if (res != result::OK) return res;										//Make sure the body index was successfully retrieved
	set<BYTE> bodyNumbers;													//Set makes sure that there is only one copy of each number ie, the numbers stored are all unique
	for (int i = 0; i < cap; i++)
		bodyNumbers.insert(buf[i]);
	bodyCount = bodyNumbers.size() - 1;										//The size of bodyNumber would be the number of unique numbers. Leave one for 0(background)
	delete[] buf;															//Don't need the buffer anymore
	return result::OK;
}

int KinectProvider::getDepthMap(OUT UINT16 **image, OUT UINT &arraySize, bool copy) {
	if (!depthFrameReader) throw error::DepthMapCaptureNotStarted;
	IDepthFrame *IDF = nullptr;
	int hr = depthFrameReader->AcquireLatestFrame(&IDF);
	if (hr != 0) return result::NotReady;
	IDF->AccessUnderlyingBuffer(&arraySize, image);
	if (copy) {
		*image = new UINT16[arraySize];
		IDF->CopyFrameDataToArray(arraySize, *image);
	}
	IDF->Release();
	return result::OK;
}

int KinectProvider::getBodyMap(OUT BYTE **map, OUT UINT &arraySize, bool copy) {
	if (!bodyMapReader) throw error::BodyMapCaptureNotStarted;
	IBodyIndexFrame *IBIF = nullptr;
	int hr = bodyMapReader->AcquireLatestFrame(&IBIF);
	if (hr != 0) return result::NotReady;
	IBIF->AccessUnderlyingBuffer(&arraySize, map);
	if (copy) {
		*map = new BYTE[arraySize];
		IBIF->CopyFrameDataToArray(arraySize, *map);
	}
	IBIF->Release();
	return result::OK;
}

int KinectProvider::getBodyData(OUT IBody **bodies) {
	if (!bodyFrameReader) throw error::BodyDataCaptureNotStarted;
	IBodyFrame *IBF = nullptr;
	int hr = bodyFrameReader->AcquireLatestFrame(&IBF);
	if (hr != 0) return result::NotReady;
	IBF->GetAndRefreshBodyData(BODY_COUNT, bodies);
	IBF->Release();
	return result::OK;
}

void KinectProvider::processAudioData() {
	IAudioBeamFrameList *IABFL = nullptr;
	audioBeamReader->AcquireLatestBeamFrames(&IABFL);
	UINT frmCount = 0;
	IABFL->get_BeamCount(&frmCount);
	for (int i = 0; i < frmCount; i++) {
		IAudioBeamFrame *IABF = nullptr;
		IABFL->OpenAudioBeamFrame(i, &IABF);
		//IABF->
	}
}