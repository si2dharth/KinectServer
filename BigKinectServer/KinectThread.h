#pragma once
#include "KinectProvider.h"
#include "TCP.h"
#include <list>
#include <set>
#include <mutex>
using namespace std;

void setKinectProvider(KinectProvider* KP);

void calConv();

class KinectThread {
	list<Client*> connectedClients;
public:
	KinectThread();
	~KinectThread();
	virtual void run() = 0;
	void sendToClient();
	void connectClient(Client* C);
};

class ImageThread : public KinectThread {
	void *image = 0;
	UINT capacity = 0;
protected:
	mutex lck;
	virtual void* collectImage(UINT &cap) = 0;
public:
	ImageThread();
	~ImageThread();
	virtual void run();
	virtual void getImage(void **image, UINT &capacity);
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