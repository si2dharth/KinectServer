#include "Server.h"
#include "KinectThread.h"

ColorImageThread *CIT = nullptr;
InfraredImageThread *IIT = nullptr;
DepthMapThread *DMT = nullptr;
BodyMapThread *BMT = nullptr;

void initAll(KinectProvider *kinect) {
	setKinectProvider(kinect);
}

void ColorImageServer(Client *C) {
	if (CIT == nullptr) {
		CIT = new ColorImageThread();
		DWORD tID;
		CreateThread(0, 0, startThread, CIT, 0, &tID);
	}
	CIT->connectClient(C);
}

void InfraredImageServer(Client *C) {
	if (!IIT) {
		IIT = new InfraredImageThread();
		DWORD tID;
		CreateThread(0, 0, startThread, IIT, 0, &tID);
	}
	IIT->connectClient(C);
}

void DepthMapServer(Client *C) {
	if (!DMT) {
		DMT = new DepthMapThread();
		DWORD tID;
		CreateThread(0, 0, startThread, DMT, 0, &tID);
	}
	DMT->connectClient(C);
}

void BodyMapServer(Client *C) {
	if (!BMT) {
		BMT = new BodyMapThread();
		DWORD tID;
		CreateThread(0, 0, startThread, BMT, 0, &tID);
	}
	BMT->connectClient(C);
}