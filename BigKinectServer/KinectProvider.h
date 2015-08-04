#include "stdafx.h"

class KinectProvider {
	IKinectSensor *sensor;

	IAudioBeamFrameReader *audioBeamReader;			//Returns various frames each time, with PCM values at 16 kHz and direction in each frame
	IColorFrameReader *colorFrameReader;			//Returns YUV2 image
	IBodyFrameReader *bodyFrameReader;				//Returns body joint data
	IBodyIndexFrameReader *bodyMapReader;			//Returns BYTE map
	IDepthFrameReader *depthFrameReader;			//Returns UINT16 image
	IInfraredFrameReader *infraredFrameReader;		//Returns UINT16 image

public:
	KinectProvider();
	~KinectProvider();

	void startColorCapture(bool infrared);
	void stopColorCapture();
	void startDepthMapCapture();
	void stopDepthMapCapture();
	void startBodyMapCapture();
	void stopBodyMapCapture();
	void startBodyDataCapture();
	void stopBodyDataCapture();
	void startAudioCapture();
	void stopAudioCapture();

	int getImage(OUT BYTE **image, OUT UINT &arraySize);
	int getInfraredImage(OUT UINT16 **image, OUT UINT &arraySize);
	int getBodyCount(OUT int &bodyCount);
	int getBodyData(OUT IBody **bodies);
	int getDepthMap(OUT UINT16 **image, OUT UINT &arraySize);
	int getBodyMap(OUT BYTE **map, OUT UINT &arraySize);
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