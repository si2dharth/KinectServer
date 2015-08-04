#include "KinectProvider.h"
#include <set>
using namespace std;

KinectProvider::KinectProvider() :
	sensor(nullptr),
	audioBeamReader(nullptr),
	colorFrameReader(nullptr),
	bodyFrameReader(nullptr),
	bodyMapReader(nullptr),
	depthFrameReader(nullptr),
	infraredFrameReader(nullptr)
{
	int hr = GetDefaultKinectSensor(&sensor);
	if (hr != 0) {
		sensor = nullptr;
		throw error::SensorNotFound;
	}
	hr = sensor->Open();
	if (hr != 0) {
		sensor->Release();
		throw error::SensorNotReady;
	}
}

KinectProvider::~KinectProvider() {
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
		IColorFrameSource *ICS = nullptr;
		int hr = sensor->get_ColorFrameSource(&ICS);
		if (hr != 0) throw error::ColorSourceNotReady;
		hr = ICS->OpenReader(&colorFrameReader);
		if (hr != 0) {
			colorFrameReader = nullptr;
			throw error::CouldNotOpenColorReader;
		}
	}
	imageUsers++;
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
		IInfraredFrameSource *IFS = nullptr;
		int hr = sensor->get_InfraredFrameSource(&IFS);
		if (hr != 0) throw error::InfraredSourceNotReady;
		hr = IFS->OpenReader(&infraredFrameReader);
		if (hr != 0) {
			infraredFrameReader = nullptr;
			throw error::CouldNotOpenInfraredReader;
		}
	}
	infraredUsers++;
}

void KinectProvider::stopInfraredCapture() {
	if (!infraredFrameReader) return;
	if (--infraredUsers > 0) return;
	infraredFrameReader->Release();
	infraredFrameReader = nullptr;
}

void KinectProvider::startBodyDataCapture() {
	if (!bodyFrameReader) {
		IBodyFrameSource *IBS = nullptr;
		int hr = sensor->get_BodyFrameSource(&IBS);
		if (hr != 0) throw error::BodyDataSourceNotReady;
		hr = IBS->OpenReader(&bodyFrameReader);
		IBS->Release();
		if (hr != 0) {
			bodyFrameReader = nullptr;
			throw error::CouldNotOpenBodyDataReader;
		}
	}
	bodyUsers++;
}

void KinectProvider::stopBodyDataCapture() {
	if (!bodyFrameReader) return;
	if (--bodyUsers > 0) return;
	bodyFrameReader->Release();
	bodyFrameReader = nullptr;
}

void KinectProvider::startDepthMapCapture() {
	if (!depthFrameReader) {
		IDepthFrameSource *IDS = nullptr;
		int hr = sensor->get_DepthFrameSource(&IDS);
		if (hr != 0) throw error::DepthMapSourceNotReady;
		hr = IDS->OpenReader(&depthFrameReader);
		IDS->Release();
		if (hr != 0) {
			depthFrameReader = nullptr;
			throw error::CouldNotOpenDepthMapReader;
		}
	}
	depthUsers++;
}

void KinectProvider::stopDepthMapCapture() {
	if (!depthFrameReader) return;
	if (--depthUsers) return;
	depthFrameReader->Release();
	depthFrameReader = nullptr;
}

void KinectProvider::startBodyMapCapture() {
	if (!bodyMapReader) {
		IBodyIndexFrameSource *IBIS = nullptr;
		int hr = sensor->get_BodyIndexFrameSource(&IBIS);
		if (hr != 0) throw error::BodyMapSourceNotReady;
		hr = IBIS->OpenReader(&bodyMapReader);
		IBIS->Release();
		if (hr != 0) {
			bodyMapReader = nullptr;
			throw error::CouldNotOpenBodyMapReader;
		}
	}
	bodyMapUsers++;
}

void KinectProvider::stopBodyMapCapture() {
	if (!bodyMapReader) return;
	if (--bodyMapUsers) return;
	bodyMapReader->Release();
	bodyMapReader = nullptr;
}

void KinectProvider::startAudioCapture() {
	if (!audioBeamReader) {
		IAudioSource *IAS = nullptr;
		int hr = sensor->get_AudioSource(&IAS);
		if (hr != 0) throw error::AudioSourceNotReady;
		hr = IAS->OpenReader(&audioBeamReader);
		IAS->Release();
		if (hr != 0) {
			audioBeamReader = nullptr;
			throw error::CouldNotOpenAudioReader;
		}
	}
	audioUsers++;
}

void KinectProvider::stopAudioCapture() {
	if (!audioBeamReader) return;
	if (--audioUsers) return;
	audioBeamReader->Release();
	audioBeamReader = nullptr;
}



int KinectProvider::getImage(OUT BYTE **image, OUT UINT &arraySize) {
	if (!colorFrameReader) throw error::ColorCaptureNotStarted;
	IColorFrame *ICF = nullptr;
	int hr = colorFrameReader->AcquireLatestFrame(&ICF);
	if (hr != 0) return result::NotReady;
	ICF->AccessRawUnderlyingBuffer(&arraySize, image);
	ICF->Release();
	return result::OK;
}

int KinectProvider::getInfraredImage(OUT UINT16 **image, OUT UINT &arraySize) {
	if (!infraredFrameReader) throw error::InfraredCaptureNotStarted;
	IInfraredFrame *IIF = nullptr;
	int hr = infraredFrameReader->AcquireLatestFrame(&IIF);
	if (hr != 0) return result::NotReady;
	IIF->AccessUnderlyingBuffer(&arraySize, image);
	IIF->Release();
	return result::OK;
}


int KinectProvider::getBodyCount(OUT int &bodyCount) {
	UINT cap = 0;
	BYTE *buf;
	int res = getBodyMap(&buf, cap);
	if (res != result::OK) return res;
	set<BYTE> bodyNumbers;
	for (int i = 0; i < cap; i++)
		bodyNumbers.insert(buf[i]);
	bodyCount = bodyNumbers.size();
	return result::OK;
}

int KinectProvider::getDepthMap(OUT UINT16 **image, OUT UINT &arraySize) {
	if (!depthFrameReader) throw error::DepthMapCaptureNotStarted;
	IDepthFrame *IDF = nullptr;
	int hr = depthFrameReader->AcquireLatestFrame(&IDF);
	if (hr != 0) return result::NotReady;
	IDF->AccessUnderlyingBuffer(&arraySize, image);
	IDF->Release();
	return result::OK;
}

int KinectProvider::getBodyMap(OUT BYTE **map, OUT UINT &arraySize) {
	if (!bodyMapReader) throw error::BodyMapCaptureNotStarted;
	IBodyIndexFrame *IBIF = nullptr;
	int hr = bodyMapReader->AcquireLatestFrame(&IBIF);
	if (hr != 0) return result::NotReady;
	IBIF->AccessUnderlyingBuffer(&arraySize, map);
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