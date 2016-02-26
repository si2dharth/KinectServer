#include "KinectProvider.h"
#include <set>
using namespace std;

KinectProvider::KinectProvider() :		//Initialize all to nullptr
	sensor(nullptr),
	audioBeamList(nullptr),
	colorFrameReader(nullptr),
	bodyFrameReader(nullptr),
	bodyMapReader(nullptr),
	depthFrameReader(nullptr),
	infraredFrameReader(nullptr)
{
	debugGen << "Starting up the Kinect\n";
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
	if (audioBeamList) audioBeamList->Release();
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
		debugGen << "Starting color capture\n";
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
		debugGen << "Stopping color capture\n";
		colorFrameReader->Release();
		colorFrameReader = nullptr;
	}
}

void KinectProvider::startInfraredCapture() {							
	if (!infraredFrameReader) {
		debugGen << "Starting infrared capture\n";
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
	debugGen << "Stopping infrared capture\n";
	infraredFrameReader->Release();
	infraredFrameReader = nullptr;
}

void KinectProvider::startBodyDataCapture() {
	if (!bodyFrameReader) {
		//Standard method: Get the source, get reader from source
		debugGen << "Starting body data capture\n";
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
	debugGen << "Stopping body data capture\n";
	bodyFrameReader->Release();
	bodyFrameReader = nullptr;
}

void KinectProvider::startDepthMapCapture() {
	if (!depthFrameReader) {
		//Standard method: Get the source, get reader from source
		debugGen << "Starting depth map capture\n";
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
	debugGen << "Stopping depth map capture\n";
	depthFrameReader->Release();
	depthFrameReader = nullptr;
}

void KinectProvider::startBodyMapCapture() {
	if (!bodyMapReader) {
		//Standard method: Get the source, get reader from source
		debugGen << "Starting body map capture\n";
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
	debugGen << "Stopping body map capture\n";
	bodyMapReader->Release();
	bodyMapReader = nullptr;
}

void KinectProvider::startAudioCapture() {
	if (!audioBeamList) {
		//Standard method: Get the source, get reader from source
		debugGen << "Starting audio capture\n";
		IAudioSource *IAS = nullptr;
		int hr = sensor->get_AudioSource(&IAS);
		if (hr != 0) throw error::AudioSourceNotReady;
		hr = IAS->get_AudioBeams(&audioBeamList);
		
		IAS->Release();										   //Don't need the source anymore
		if (hr != 0) {
			audioBeamList = nullptr;						   //Make sure its not set to an arbitrary value in case of failure
			throw error::CouldNotOpenAudioReader;
		}
	}
	audioUsers++;											   //Count the number of users for the reader.
}

void KinectProvider::stopAudioCapture() {
	if (!audioBeamList) return;
	if (--audioUsers) return;
	debugGen << "Stopping audio capture\n";
	audioBeamList->Release();
	audioBeamList = nullptr;
}



int KinectProvider::getImage(OUT BYTE **image, OUT UINT &arraySize, bool copy) {
	if (!colorFrameReader) throw error::ColorCaptureNotStarted;			//Make sure that capture was started
	debugGen << "Trying to get an image...";
	IColorFrame *ICF = nullptr;
	int hr = colorFrameReader->AcquireLatestFrame(&ICF);
	if (hr != 0) {
		debugGen << "Image not ready\n";
		return result::NotReady;
	}
	ICF->AccessRawUnderlyingBuffer(&arraySize, image);					//Access buffer regardless of copy parameter as arraySize is required
	if (copy) {
		*image = new BYTE[arraySize];									//Kinect does not manage array
		ICF->CopyRawFrameDataToArray(arraySize, *image);				
	}
	ICF->Release();
	debugGen << "Done\n";
	return result::OK;
}

int KinectProvider::getInfraredImage(OUT UINT16 **image, OUT UINT &arraySize, bool copy) {
	if (!infraredFrameReader) throw error::InfraredCaptureNotStarted;		//Make sure that capture was started
	debugGen << "Trying to get an infrared image...";
	IInfraredFrame *IIF = nullptr;
	int hr = infraredFrameReader->AcquireLatestFrame(&IIF);
	if (hr != 0) {
		debugGen << "Infrared image not ready\n";
		return result::NotReady;
	}
	IIF->AccessUnderlyingBuffer(&arraySize, image);							//Access buffer regardless of copy parameter as arraySize is required
	if (copy) {
		*image = new UINT16[arraySize];										//Kinect does not manage array
		IIF->CopyFrameDataToArray(arraySize, *image);
	}
	IIF->Release();
	debugGen << "Done\n";
	return result::OK;
}


int KinectProvider::getBodyCount(OUT int &bodyCount) {
	UINT cap = 0;
	BYTE *buf;
	int res = getBodyMap(&buf, cap, true);									//Try to get the body index map
	if (res != result::OK) return res;										//Make sure the body index was successfully retrieved
	set<BYTE> bodyNumbers;													//Set makes sure that there is only one copy of each number ie, the numbers stored are all unique
	for (UINT i = 0; i < cap; i++)
		bodyNumbers.insert(buf[i]);
	bodyCount = bodyNumbers.size() - 1;										//The size of bodyNumber would be the number of unique numbers. Leave one for 0(background)
	delete[] buf;															//Don't need the buffer anymore
	return result::OK;
}

int KinectProvider::getDepthMap(OUT UINT16 **image, OUT UINT &arraySize, bool copy) {
	if (!depthFrameReader) throw error::DepthMapCaptureNotStarted;
	debugGen << "Trying to get depth map frame...";
	IDepthFrame *IDF = nullptr;
	int hr = depthFrameReader->AcquireLatestFrame(&IDF);
	if (hr != 0) {
		debugGen << "Depth map not ready\n";
		return result::NotReady;
	}
	IDF->AccessUnderlyingBuffer(&arraySize, image);
	if (copy) {
		*image = new UINT16[arraySize];
		IDF->CopyFrameDataToArray(arraySize, *image);
	}
	IDF->Release();
	debugGen << "Done\n";
	return result::OK;
}

int KinectProvider::getBodyMap(OUT BYTE **map, OUT UINT &arraySize, bool copy) {
	if (!bodyMapReader) throw error::BodyMapCaptureNotStarted;
	debugGen << "Trying to get body map frame...";
	IBodyIndexFrame *IBIF = nullptr;
	int hr = bodyMapReader->AcquireLatestFrame(&IBIF);
	if (hr != 0) {
		debugGen << "Body frame not ready\n";
		return result::NotReady;
	}
	IBIF->AccessUnderlyingBuffer(&arraySize, map);
	if (copy) {
		*map = new BYTE[arraySize];
		IBIF->CopyFrameDataToArray(arraySize, *map);
	}
	IBIF->Release();
	debugGen << "Done\n";
	return result::OK;
}

int nBodies = 0;
int KinectProvider::getBodyData(OUT bool trackStates[], OUT Joint joints[][JointType_Count], OUT HandState handStates[][2]) {
	if (!bodyFrameReader) throw error::BodyDataCaptureNotStarted;
	debugGen << "Trying to get body data...";
	IBodyFrame *IBF = nullptr;
	int hr = bodyFrameReader->AcquireLatestFrame(&IBF);
	if (hr != 0) {
		debugGen << "Body frame not ready\n";
		return result::NotReady;
	}
	IBody *bodies[BODY_COUNT] = { 0 };
	IBF->GetAndRefreshBodyData(BODY_COUNT, bodies);
	for (int i = 0; i < BODY_COUNT; ++i) {
		BOOLEAN isTracked = false;
		if (bodies[i]->get_IsTracked(&isTracked) != 0) {
			debugGen << "Body frame not ready\n";
			return result::NotReady;
		}

		if (!isTracked) {
			trackStates[i] = false;
			handStates[i][0] = HandState_NotTracked;
			handStates[i][1] = HandState_NotTracked;
		}
		else {
			trackStates[i] = true;
			bodies[i]->GetJoints(JointType_Count, joints[i]);
			bodies[i]->get_HandLeftState(&handStates[i][0]);
			bodies[i]->get_HandRightState(&handStates[i][1]);
		}
		bodies[i]->Release();
	}
	IBF->Release();
	debugGen << "Done\n";
	return result::OK;
}

int KinectProvider::getAudioData(int index, OUT IStream *&stream) {
	if (!audioBeamList) throw error::AudioCaptureNotStarted;
	debugGen << "Trying to get audio data...";
	IAudioBeam *audioBeam = nullptr;
	HRESULT hr;
	hr = audioBeamList->OpenAudioBeam(index, &audioBeam);
	if (hr != 0) {
		debugGen << "Audio data not ready\n";
		return result::NotReady;
	}
	hr = audioBeam->OpenInputStream(&stream);
	if (hr != 0) return result::NotReady;
	audioBeam->Release();
	debugGen << "Done\n";
	return result::OK;
}