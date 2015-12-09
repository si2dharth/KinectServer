#include "Server.h"
#include "KinectThread.h"
#include "ConnectionManager.h"

#include<string>
#include<vector>
#include <ctime>
#include <mutex>
using namespace std;
///The KinectThread that collects color images
ColorImageThread *CIT = nullptr;

///The KinectThread that collects infrared images
InfraredImageThread *IIT = nullptr;

///The KinectThread that collects depth map
DepthMapThread *DMT = nullptr;

///The KinectThread that collects body index map
BodyMapThread *BMT = nullptr;

///The KinectThread that collects body joint data
BodyThread *BT = nullptr;

///The KinectThread that processes audio and provides speech recognition
AudioThread *AT = nullptr;

int CITUsers = 0, IITUsers = 0, DMTUsers = 0, BMTUsers = 0, BTUsers = 0, ATUsers = 0;
mutex cMutex, iMutex, dMutex, bMutex, jMutex, aMutex;

void initAll(KinectProvider *kinect) {
	setKinectProvider(kinect);					//Ask the global function of the KinectThread header to set the kinect.
}

///Process commands from a given Client using the specified ImageThread
void processClientMessages(ImageThread *IT, Client *C) {
	while (true) {												//Repeat forever
		string s;
		int nChar = C->receive(s);								//Get command from client
		if (nChar <= 0) break;									//Client seems to be disconnected.
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
	OutputDebugString("Disconnected\n");
}

void ColorImageServer(Client *C) {
	cMutex.lock();
	if (CIT == nullptr) {
		CIT = new ColorImageThread();							//Create a ColorImageThread if it hasn't been created yet. This way, a thread is started only if it is required
	}
	CITUsers++;
	addConnection(C, "Color_Images");
	cMutex.unlock();
	
	processClientMessages(CIT, C);
	
	cMutex.lock();
	CITUsers--;
	if (CITUsers == 0) {
		delete CIT;
		CIT = nullptr;
	}
	removeConnection(C);
	cMutex.unlock();
}

void InfraredImageServer(Client *C) {
	iMutex.lock();
	if (!IIT) {
		IIT = new InfraredImageThread();						//Create a InfraredImageThread if it hasn't been created yet. This way, a thread is started only if it is required	
	}
	IITUsers++;
	addConnection(C, "Infrared_Images");
	iMutex.unlock();

	processClientMessages(IIT, C);

	iMutex.lock();
	IITUsers--;
	removeConnection(C);
	if (IITUsers == 0) {
		delete IIT;
		IIT = nullptr;
	}
	iMutex.unlock();
}

void DepthMapServer(Client *C) {
	dMutex.lock();
	if (!DMT) {
		DMT = new DepthMapThread();								//Create a DepthMapThread if it hasn't been created yet. This way, a thread is started only if it is required
	}
	DMTUsers++;
	addConnection(C, "Depth_Map");
	dMutex.unlock();

	processClientMessages(DMT, C);

	dMutex.lock();
	DMTUsers--;
	removeConnection(C);
	if (DMTUsers == 0) {
		delete DMT;
		DMT = nullptr;
	}
	dMutex.unlock();
}

void BodyMapServer(Client *C) {
	bMutex.lock();
	if (!BMT) {
		BMT = new BodyMapThread();								//Create a BodyMapThread if it hasn't been created yet. This way, a thread is started only if it is required
	}
	BMTUsers++;
	addConnection(C,"Body_Map");
	bMutex.unlock();

	processClientMessages(BMT, C);

	bMutex.lock();
	BMTUsers--;
	removeConnection(C);
	if (BMTUsers == 0) {
		delete BMT;
		BMT = nullptr;
	}
	bMutex.unlock();
}

map<string, JointType> jointNames = {
	{"SpineBase",JointType_SpineBase},
	{"SpineMid",JointType_SpineMid},
	{"Neck",JointType_Neck},
	{"Head", JointType_Head},
	{"ShoulderLeft",JointType_ShoulderLeft},
	{"ElbowLeft",JointType_ElbowLeft},
	{"WristLeft", JointType_WristLeft},
	{"HandLeft",JointType_HandLeft},
	{"ShoulderRight",JointType_ShoulderRight},
	{"ElbowRight",JointType_ElbowRight},
	{"WristRight",JointType_WristRight},
	{"HandRight",JointType_HandRight},
	{"HipLeft",JointType_HipLeft},
	{"KneeLeft",JointType_KneeLeft},
	{"AnkleLeft",JointType_AnkleLeft},
	{"FootLeft",JointType_FootLeft},
	{"HipRight",JointType_HipRight},
	{"KneeRight",JointType_KneeRight},
	{"AnkleRight",JointType_AnkleRight},
	{"FootRight",JointType_FootRight},
	{"SpineShoulder",JointType_SpineShoulder},
	{"HandTipLeft",JointType_HandTipLeft},
	{"ThumbLeft",JointType_ThumbLeft},
	{"HandTipRight",JointType_HandTipRight},
	{"ThumbRight",JointType_ThumbRight}
};



int Clip(int i) {
	if (i < 0) return 0;
	if (i > 65535) return 65535;
	return i;
}

void BodyServer(Client *C) {
	jMutex.lock();
	if (!BT) {
		BT = new BodyThread();
		Sleep(1000);
	}
	BTUsers++;
	addConnection(C, "Skeleton_Data");
	jMutex.unlock();
	C->disableNagles();
	clock_t lastSendTime = clock();
	string s;
	C->receive(s);
	vector<string> strs = split(s);

	while (true) {		
		while (clock() - lastSendTime < 16)
			Sleep(clock() - lastSendTime);
		OutputDebugString((to_string(clock() - lastSendTime) + "\n"s).c_str());
		lastSendTime = clock();
		if (strs[0] == "get") {
			int i = 1;
			vector<char> output;
			while (strs.size() > i)
			{
				JointType query;
				if (isalpha(strs[i+1][0]))
					query = jointNames[strs[i+1]];
				else
					query = (JointType)stoi(strs[i+1]);

				Joint J;
				BT->getJoint(&J, stoi(strs[i]), query);
				
				switch (J.TrackingState) {
				case TrackingState_Inferred:
					output.push_back('I');
					break;
				case TrackingState_NotTracked:
					output.push_back('N');
					break;
				case TrackingState_Tracked:
					output.push_back('T');
					break;
				}
				if (query == JointType_HandLeft) {
					bool closed;
					BT->getHandState(&closed, stoi(strs[1]), 1);
					output.push_back(closed ? 'C' : 'O');
				}
				else if (query == JointType_HandRight) {
					bool closed;
					BT->getHandState(&closed, stoi(strs[1]), 2);
					output.push_back(closed ? 'C' : 'O');
				}
				int x, y,z;
				x = Clip(32767 + (int)(J.Position.X * 32768 / 0.8));
				y = Clip(32767 + (int)(J.Position.Y * 32768 / 0.6));
				z = Clip(32767 + (int)(J.Position.Z * 32768 / 5.00));
				output.push_back((x & 0x0000FF00) >> 8);
				output.push_back(x & 0x000000FF);
				output.push_back((y & 0x0000FF00) >> 8);
				output.push_back(y & 0x000000FF);
				output.push_back((z & 0x0000FF00) >> 8);
				output.push_back(z & 0x000000FF);
				i += 2;
			}



			char *outputStr = new char[output.size()];
			i = 0;
			for (auto c : output) {
				outputStr[i++] = c;
			}
		//	int success = C->send(to_string(output.size()) + "\n"s);
			int success = C->send(outputStr, output.size());
			if (success < 0) break;
			//OutputDebugString(outputStr);
		}
		else if (strs[0] == "getNum") {
			C->send(to_string(BT->getNumberOfBodies()) + "\n"s);
		}
		else if (strs[0] == "disconnect") {
			break;
		}
	}

	jMutex.lock();
	BTUsers--;
	removeConnection(C);
	if (BTUsers == 0) {
		delete BT;
		BT = nullptr;
	}
	jMutex.unlock();
}

void SpeechServer(Client *C) {
	aMutex.lock();
	if (!AT) {
		AT = new AudioThread();
	}
	ATUsers++;
	addConnection(C, "Speech_Recog");
	aMutex.unlock();

	int id = AT->registerUser();

	string inp;
	C->receive(inp);
	auto commands = split(inp);
	for (auto &s : commands) {
		AT->addPhrase(id, s);
	}

	while (true) {
		string phrase;
		if (AT->getSpokenPhrase(id, phrase)) {
			if (C->send(phrase) < 0) break;
		}
	}

	AT->unregisterUser(id);
	aMutex.lock();
	ATUsers--;
	removeConnection(C);
	if (ATUsers == 0) {
		delete AT;
		AT = nullptr;
	}
	aMutex.unlock();
}