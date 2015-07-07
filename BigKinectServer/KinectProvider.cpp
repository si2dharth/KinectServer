#include "KinectProvider.h"

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


void KinectProvider::startColorCapture(bool infrared) {
	if (infrared) {
		if (colorFrameReader) {
			colorFrameReader->Release();
			colorFrameReader = nullptr;
		}
	}
	else {
		if (infraredFrameReader) {
			infraredFrameReader->Release();
			infraredFrameReader = nullptr;
		}
	}

	///If reader is open, don't do anything
	if (infrared && infraredFrameReader) return;
	if (!infrared && colorFrameReader) return;

	if (infrared) {
		IInfraredFrameSource *IFS = nullptr;
		int hr = sensor->get_InfraredFrameSource(&IFS);
		if (hr != 0) throw error::InfraredSourceNotReady;
		hr = IFS->OpenReader(&infraredFrameReader);
		if (hr != 0) {
			infraredFrameReader = nullptr;
			throw error::CouldNotOpenInfraredReader;
		}
	}
	else {
		IColorFrameSource *ICS = nullptr;
		int hr = sensor->get_ColorFrameSource(&ICS);
		if (hr != 0) throw error::ColorSourceNotReady;
		hr = ICS->OpenReader(&colorFrameReader);
		if (hr != 0) {
			colorFrameReader = nullptr;
			throw error::CouldNotOpenColorReader;
		}
	}
}

void KinectProvider::stopColorCapture() {
	if (colorFrameReader) {
		colorFrameReader->Release();
		colorFrameReader = nullptr;
	}
	if (infraredFrameReader) {
		infraredFrameReader->Release();
		infraredFrameReader = nullptr;
	}
}

void KinectProvider::startBodyDataCapture() {
	if (bodyFrameReader) return;
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

void KinectProvider::stopBodyDataCapture() {
	if (!bodyFrameReader) return;
	bodyFrameReader->Release();
	bodyFrameReader = nullptr;
}

void KinectProvider::startBodyMapCapture() {
	if (bodyMapReader) return;
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

void KinectProvider::stopBodyMapCapture() {
	if (!bodyMapReader) return;
	bodyMapReader->Release();
	bodyMapReader = nullptr;
}

void KinectProvider::startAudioCapture() {
	if (audioBeamReader) return;
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

void KinectProvider::stopAudioCapture() {
	if (!audioBeamReader) return;
	audioBeamReader->Release();
	audioBeamReader = nullptr;
}

