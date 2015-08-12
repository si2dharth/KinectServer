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
	while (true) {
		string s;
		C->receive(s);
		if (s == "get") 
		{
			void *img = nullptr;
			UINT capacity = 0;
			while (capacity == 0 || img == nullptr)
				CIT->getImage(1, &img, capacity);
			int i = 0;
			i = C->send(to_string(capacity) + "\n"s);
			i = C->send((char*)img, capacity);
			delete[] img;
			img = 0;
			if (i < 0) break;
		}
		else if (s == "disconnect") break;
	}
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