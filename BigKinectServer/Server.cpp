#include "Server.h"
#include "KinectThread.h"

///The KinectThread that collects color images
ColorImageThread *CIT = nullptr;

///The KinectThread that collects infrared images
InfraredImageThread *IIT = nullptr;

///The KinectThread that collects depth map
DepthMapThread *DMT = nullptr;

///The KinectThread that collect body index map
BodyMapThread *BMT = nullptr;

void initAll(KinectProvider *kinect) {
	setKinectProvider(kinect);					//Ask the global function of the KinectThread header to set the kinect.
}

///Process commands from a given Client using the specified ImageThread
void processClientMessages(ImageThread *IT, Client *C) {
	while (true) {												//Repeat forever
		string s;
		C->receive(s);											//Get command from client
		if (s == "get")											//To get an image, the client sends "get"
		{
			void *img = nullptr;
			UINT capacity = 0;
			while (capacity == 0 || img == nullptr)
				IT->getImage(&img, capacity);					//Get a copy of image frame from the ImageThread
			int i = 0;
			i = C->send(to_string(capacity) + "\n"s);			//Send the size of the image in bytes followed by newline character
			i = C->send((char*)img, capacity);					//Send the entire image
			delete[] img;										//Delete the copy
			img = 0;
			if (i < 0) break;									//If sending failed, disconnect from client and stop thread.
		}
		else if (s == "disconnect") break;						//If client sends "disconnect", disconnect from client and stop thread.
	}
}

void ColorImageServer(Client *C) {
	if (CIT == nullptr) {
		CIT = new ColorImageThread();							//Create a ColorImageThread if it hasn't been created yet. This way, a thread is started only if it is required
		DWORD tID;
		CreateThread(0, 0, startThread, CIT, 0, &tID);			//Run the ColorImageThread on a new thread
	}
	processClientMessages(CIT, C);
}

void InfraredImageServer(Client *C) {
	if (!IIT) {
		IIT = new InfraredImageThread();						//Create a InfraredImageThread if it hasn't been created yet. This way, a thread is started only if it is required
		DWORD tID;
		CreateThread(0, 0, startThread, IIT, 0, &tID);			//Run the InfraredImageThread on a new thread
	}
	processClientMessages(IIT, C);
}

void DepthMapServer(Client *C) {
	if (!DMT) {
		DMT = new DepthMapThread();								//Create a DepthMapThread if it hasn't been created yet. This way, a thread is started only if it is required
		DWORD tID;
		CreateThread(0, 0, startThread, DMT, 0, &tID);			//Run the DepthMapThread on a new thread
	}
	processClientMessages(DMT, C);
}

void BodyMapServer(Client *C) {
	if (!BMT) {
		BMT = new BodyMapThread();								//Create a BodyMapThread if it hasn't been created yet. This way, a thread is started only if it is required
		DWORD tID;
		CreateThread(0, 0, startThread, BMT, 0, &tID);			//Run the BodyMapThread on a new thread
	}
	processClientMessages(BMT, C);
}