///This file provides links between TCP and KinectProvider
#pragma once
#include "KinectProvider.h"
#include "BodyFrameProvider.h"
#include "SpeechHandler.h"
#include "TCP.h"
#include <list>
#include <set>
#include <mutex>
#include <atomic>
using namespace std;

///Set the KinectProvider for the program
/**
@param [in] KP Pointer to a KinectProvider object.
*/
void setKinectProvider(KinectProvider* KP);

KinectProvider* getKinectProvider();

///Pre-calculate conversion values for YUV2 to RGB format
/**
Call this function at the start of program.
For converting a YUV2 image to a RGB image, a conversion formula has to be run for each and every pixel. 
This calculation takes a significant amount of time, especially when performed 1920*1080 times, for every frame.
So, for every value of Y,U,V from 0-255, RGB values are calculated and stored.
*/
void calConv();

///An abstract class for collecting data from the Kinect.
/**
After creation, the method run should be called repeatedly forever. The run method is supposed to collect data from the Kinect.
This method should run on its own thread so that other operations can be performed.
*/
class KinectThread {
	atomic<bool> stopThread;
	thread backgroundWorker;
protected:
	void runThread();
	static int numUsers;
public:
	///Constructor. Makes sure that setKinectProvider was called prior to creating the object otherwise throws an exception.
	KinectThread();

	//Stop thread before deleting anything
	void destroy();

	///Destructor
	virtual ~KinectThread();
	///Abstract run method. The method is meant to collect and store data.
	virtual void run() = 0;
};


///An abstract class for collecting data consisting of images/maps
/**
When run, image is captured and saved in the object. The object can then be asked, at any time, for the last saved object.
collectImage is abstract. The method is supposed to return a void pointer to an image as collected by the resepctive thread object.
*/
class ImageThread : public KinectThread {
	void *image = 0;										///<The saved image
	UINT capacity = 0;										///<The size of the image in bytes
protected:													
	mutex lck;												///<Mutex to manage image variable between different threads
	
	///Abstract function to collect image/map from the Kinect.
	/**
	The method return the size of image in cap and a pointer to the image as result
	@param [out] cap The size of image in bytes is returned in this variable
	*/
	virtual void* collectImage(UINT &cap) = 0;				
public:			
	///Constructor. Initializes the mutex.
	ImageThread();	

	///Destructor. Destroys the saved image.
	virtual ~ImageThread();	

	///Run method collects and saves images. The method is supposed to be called repeatedly on a separate thread.
	virtual void run();										

	///Get the last saved image
	/**
	@param [out] image Pointer to an array containing the image is returned 
	@param [out] capacity The size of the image in bytes
	*/
	virtual void getImage(void **image, UINT &capacity);	
};

///Extension of ImageThread. Collects color image frames from the kinect.
class ColorImageThread : public ImageThread {
protected: 
	virtual void* collectImage(UINT &cap);
public: 
	///Constructor. Starts color capture in the KinectProvider
	ColorImageThread();

	///Destructor. Stops color capture
	~ColorImageThread();

	static void initialize();
	static void finalize();
};

///Extension of ImageThread. Collects infrared image frames from the kinect.
class InfraredImageThread : public ImageThread {
protected:
	virtual void* collectImage(UINT &cap);
public:
	///Constructor. Starts infrared capture in the KinectProvider
	InfraredImageThread();

	///Destructor. Stops infrared capture
	~InfraredImageThread();

	static void initialize();
	static void finalize();
};

///Extension of ImageThread. Collects depth map frames from the kinect.
class DepthMapThread : public ImageThread {
protected:
	virtual void* collectImage(UINT &cap);
public:
	///Constructor. Starts depth map capture in the KinectProvider
	DepthMapThread();

	///Destructor. Stops depth map capture
	~DepthMapThread();

	static void initialize();
	static void finalize();
};

///Extension of ImageThread. Collects body map frames from the kinect.
class BodyMapThread : public ImageThread {
protected:
	virtual void* collectImage(UINT &cap);
public:
	///Constructor. Starts body map capture in the KinectProvider
	BodyMapThread();
	
	///Destructor. Stops body map capture
	~BodyMapThread();

	static void initialize();
	static void finalize();
};

class BodyThread : public KinectThread {
	static BodyFrameProvider* bodyFP;
public:
	BodyThread();
	~BodyThread();
	virtual void run();

	int getJoint(Joint *J, int bodyNumber, JointType jointType);
	int getHandState(bool *closed, int bodyNumber, int side);
	int getNumberOfBodies();

	static void initialize();
	static void finalize();
};

class AudioThread : public KinectThread {
	static SpeechProvider *speechP;
	map<int, set<string>> phraseDict;
	map<int, queue<string>> spokenWords;
	mutex edit, q;

	int curUser;
public:
	AudioThread();
	virtual ~AudioThread();
	virtual void run();

	int registerUser();
	void unregisterUser(int userID);
	void addPhrase(int userID, string phrase);
	void addPhrases(int userID, vector<string> phrases);
	void setGrammar(int userID, string grammar);
	void setGrammar(int userID, vector<string> grammar, int startIndex);
	bool getSpokenPhrase(int userID, string &phrase);

	static void initialize();
	static void finalize();
};