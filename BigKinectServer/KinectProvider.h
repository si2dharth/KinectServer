#pragma once
#include "stdafx.h"

///KinectProvider provides accessibility to Kinect directly using functions without worrying about interfaces. It also maintains the state of Kinect by itself.
class KinectProvider {
	IKinectSensor *sensor;							///<The pointer to the IKinectSensor object used by the object
													
	int audioUsers = 0,								///<The number of users using audio beams
		imageUsers = 0, 							///<The number of users using color images
		infraredUsers = 0, 							///<The number of users using infrared images
		bodyUsers = 0, 								///<The number of users using body joint data
		bodyMapUsers = 0, 							///<The number of users using body maps
		depthUsers = 0;								///<The number of users using depth maps

	IAudioBeamFrameReader *audioBeamReader;			///<The reader for audioBeams .Returns various frames each time, with PCM values at 16 kHz and direction in each frame
	IColorFrameReader *colorFrameReader;			///<The reader for color image frames. Returns YUV2 image
	IBodyFrameReader *bodyFrameReader;				///<The reader for body joint data frames. Returns body joint data
	IBodyIndexFrameReader *bodyMapReader;			///<The reader for body index map frames. Returns BYTE map
	IDepthFrameReader *depthFrameReader;			///<The reader for depth map frames. Returns UINT16 image
	IInfraredFrameReader *infraredFrameReader;		///<The reader for infrared frames. Returns UINT16 image

public:
	///The constructor. Finds a connected Kinect and starts interface with it.
	KinectProvider();								

	///Disconnects from the kinect
	~KinectProvider();
	
	///Start capturing color images
	void startColorCapture();

	///Stop capturing color images. Capturing will stop only if stop is called as many times as start was called
	void stopColorCapture();

	///Start capturing infrared images
	void startInfraredCapture();

	///Stop capturing infrared images
	void stopInfraredCapture();

	///Start capturing depth map frames
	void startDepthMapCapture();

	///Stop capturing depth map frames
	void stopDepthMapCapture();

	///Start capturing body index map frames
	void startBodyMapCapture();

	///Stop capturing body index map frames
	void stopBodyMapCapture();

	///Start capturing body joint data
	void startBodyDataCapture();

	///Stop capturing body joint data
	void stopBodyDataCapture();

	///Start capturing audio data
	void startAudioCapture();

	///Stop capturing audio data
	void stopAudioCapture();

	///Get color image in YUV2 format
	/**
	Gets the current color image frame from the Kinect sensor. startColorCapture() should have been called before this method is called otherwise exception KinectProvider::error.ColorCaptureNotStarted will be thrown.
	It is possible that the sensor does not have a new frame after the last frame was collected. In this case, KinectProvider::result.NotReady will be returned.
	If the image is captured properly, KinectProvider::result.OK will be returned.
	
	@param [out] image The image is returned in this BYTE array. The pointer to the array is to be passed.
	@param [out] arraySize The parameter will hold the size of the array if the image was captured without errors.
	@param [in] copy If true, the image is copied to the BYTE array from the Kinect sensor. If false, a direct access to Kinect sensor's buffer is returned. The Kinect's buffer becomes invalid as soon as a new frame is captured.
	*/
	int getImage(OUT BYTE **image, OUT UINT &arraySize, bool copy = false);

	///Get infrared image in terms of infrared values
	/**
	Gets the current infrared image frame from the Kinect sensor. startInfraredCapture() should have been called before this method is called otherwise exception KinectProvider::error.InfraredCaptureNotStarted will be thrown.
	It is possible that the sensor does not have a new frame after the last frame was collected. In this case, KinectProvider::result.NotReady will be returned.
	If the image is captured properly, KinectProvider::result.OK will be returned.
	The image returned is an array of UINT16 values. Each value contains the infrared value for the specific pixel.

	@param [out] image The image is returned in this UINT16 array. The pointer to the array is to be passed.
	@param [out] arraySize The parameter will hold the size of the array if the image was captured without errors.
	@param [in] copy If true, the image is copied to the BYTE array from the Kinect sensor. If false, a direct access to Kinect sensor's buffer is returned. The Kinect's buffer becomes invalid as soon as a new frame is captured.
	*/
	int getInfraredImage(OUT UINT16 **image, OUT UINT &arraySize, bool copy = false);

	///Get the number of bodies in front of the sensor.
	/**
	The function returns KinectProvider::result.OK if the body count was successfully determined, otherwise returns KinectProvider::result.NotReady.
	startBodyMapCapture() should have been called before this method is called otherwise exception KinectProvider::error.BodyMapCaptureNotStarted will be thrown.

	@param [out] bodyCount The number of bodies in front of the sensor is returned in this parameter.
	*/
	int getBodyCount(OUT int &bodyCount);

	///Get body joint data for all bodies in front of sensor
	/**
	Gets the current body joint data frame from the Kinect sensor. startBodyDataCapture() should have been called before this method is called otherwise exception KinectProvider::error.BodyDataCaptureNotStarted will be thrown.
	The array returned contains IBody interface. It is recommended to use BodyFrameProvider instead of using this function directly.
	The method returns KinectProvider::result.OK if the frame was successfully captured otherwise KinectProvider::result.NotReady is returned
	@param [out] bodies A pointer to an array of IBody. The array of IBody is returned in this
	*/
	int getBodyData(OUT IBody **bodies);


	int getDepthMap(OUT UINT16 **image, OUT UINT &arraySize, bool copy = false);
	int getBodyMap(OUT BYTE **map, OUT UINT &arraySize, bool copy = false);
	
	///Not implemented
	int getAudioData(int index, OUT IStream *stream);

	///Not implemented
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