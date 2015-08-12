#pragma once
#include "KinectProvider.h"
#include "TCP.h"
#include <list>
#include <set>
using namespace std;

void setKinectProvider(KinectProvider* KP);

class KinectThread {
	list<Client*> connectedClients;
public:
	KinectThread();
	~KinectThread();
	virtual void run() = 0;
	virtual void getDataToSend(char **c, int &length) = 0;
	void sendToClient();
	void connectClient(Client* C);
};

class ImageThread : public KinectThread {
	unsigned decayTime = 0;
	list<void*> images;
	multiset<int> times;
	UINT capacity = 0;
	void calculateDecayTime();
protected:
	bool lock = false;
	virtual void* collectImage(UINT &cap) = 0;
public:
	ImageThread();
	~ImageThread();
	virtual void run();
	virtual void getImage(int time, void **image, UINT &capacity);
	virtual void getDataToSend(char **c, int &length);
	virtual void setDecay(unsigned newTime);
	virtual void removeDecay(unsigned time);
};

class ColorImageThread : public ImageThread {
protected: 
	virtual void* collectImage(UINT &cap);
public: 
	ColorImageThread();
	~ColorImageThread();
};

class InfraredImageThread : public ImageThread {
protected:
	virtual void* collectImage(UINT &cap);
public:
	InfraredImageThread();
	~InfraredImageThread();
};

class DepthMapThread : public ImageThread {
protected:
	virtual void* collectImage(UINT &cap);
public:
	DepthMapThread();
	~DepthMapThread();
};

class BodyMapThread : public ImageThread {
protected:
	virtual void* collectImage(UINT &cap);
public:
	BodyMapThread();
	~BodyMapThread();
};

class BodyThread : public KinectThread {

};

DWORD __stdcall startThread(void *kinectThread);