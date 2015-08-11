#pragma once
#include "stdafx.h"

class KinectProvider {
	IKinectSensor *sensor;

	int audioUsers = 0, imageUsers = 0, infraredUsers = 0, bodyUsers = 0, bodyMapUsers = 0, depthUsers = 0;

	IAudioBeamFrameReader *audioBeamReader;			//Returns various frames each time, with PCM values at 16 kHz and direction in each frame
	IColorFrameReader *colorFrameReader;			//Returns YUV2 image
	IBodyFrameReader *bodyFrameReader;				//Returns body joint data
	IBodyIndexFrameReader *bodyMapReader;			//Returns BYTE map
	IDepthFrameReader *depthFrameReader;			//Returns UINT16 image
	IInfraredFrameReader *infraredFrameReader;		//Returns UINT16 image

public:
	KinectProvider();
	~KinectProvider();

	void startColorCapture();
	void stopColorCapture();
	void startInfraredCapture();
	void stopInfraredCapture();
	void startDepthMapCapture();
	void stopDepthMapCapture();
	void startBodyMapCapture();
	void stopBodyMapCapture();
	void startBodyDataCapture();
	void stopBodyDataCapture();
	void startAudioCapture();
	void stopAudioCapture();

	int getImage(OUT BYTE **image, OUT UINT &arraySize, bool copy = false);
	int getInfraredImage(OUT UINT16 **image, OUT UINT &arraySize, bool copy = false);
	int getBodyCount(OUT int &bodyCount);
	int getBodyData(OUT IBody **bodies);
	int getDepthMap(OUT UINT16 **image, OUT UINT &arraySize, bool copy = false);
	int getBodyMap(OUT BYTE **map, OUT UINT &arraySize, bool copy = false);
	int getAudioData(int index, OUT IStream *stream);

	void processAudioData();



	enum error {
		SensorNotFound,
		SensorNotReady,
		
		InfraredSourceNotReady,
		CouldNotOpenInfraredReader,
		InfraredCaptureNotStarted,

		ColorSourceNotReady,
		CouldNotOpenColorReader,
		ColorCaptureNotStarted,

		BodyMapSourceNotReady,
		CouldNotOpenBodyMapReader,
		BodyMapCaptureNotStarted,

		DepthMapSourceNotReady,
		CouldNotOpenDepthMapReader,
		DepthMapCaptureNotStarted,

		BodyDataSourceNotReady,
		CouldNotOpenBodyDataReader,
		BodyDataCaptureNotStarted,

		AudioSourceNotReady,
		CouldNotOpenAudioReader,
		AudioCaptureNotStarted,

		UnknownError,
	};

	enum result {
		NotReady,
		OK,
	};
};