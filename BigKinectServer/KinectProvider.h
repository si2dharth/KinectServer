#include "stdafx.h"

class KinectProvider {
	IKinectSensor *sensor;

	IAudioBeamFrameReader *audioBeamReader;
	IColorFrameReader *colorFrameReader;
	IBodyFrameReader *bodyFrameReader;
	IBodyIndexFrameReader *bodyMapReader;
	IDepthFrameReader *depthFrameReader;
	IInfraredFrameReader *infraredFrameReader;

public:
	KinectProvider();
	~KinectProvider();

	void startColorCapture(bool infrared);
	void stopColorCapture();
	void startBodyMapCapture();
	void stopBodyMapCapture();
	void startBodyDataCapture();
	void stopBodyDataCapture();
	void startAudioCapture();
	void stopAudioCapture();

	int getImage(OUT BYTE **image, OUT UINT &arraySize);
	int getInfraredImage(OUT UINT16 **image, OUT UINT &arraySize);
	int getBodyCount();
	int getBodyData();
	int getDepthMap();
	int getBodyMap();
	int getAudioData();



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