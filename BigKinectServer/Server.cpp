#include "Server.h"
#include "KinectThread.h"
#include "ConnectionManager.h"
#include "DebugProvider.h"

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
		if (nChar <= 0) {											//Client seems to be disconnected.
			debugGen << C->getIP() << " disconnected\n";
			break;
		}
		if (s == "get")											//To get an image, the client sends "get"
		{
			debugGen << C->getIP() << " : get\n";
			void *img = nullptr;
			UINT capacity = 0;
			while (capacity == 0 || img == nullptr)
				IT->getImage(&img, capacity);					//Get a copy of image frame from the ImageThread
			int i = 0;
			i = C->send(to_string(capacity) + "\n"s);			//Send the size of the image in bytes followed by newline character
			i = C->send((char*)img, capacity);					//Send the entire image
			delete[] img;										//Delete the copy
			img = 0;
			if (i < 0) {										//If sending failed, disconnect from client and stop thread.
				debugGen << C->getIP() << " disconnected\n";
				break;
			}
		}
		else if (s == "disconnect") break;						//If client sends "disconnect", disconnect from client and stop thread.
	}
	OutputDebugString("Disconnected\n");
}

void ColorImageServer(Client *C) {
	debugGen << C->getIP() << " successfully connected to Color image server\n";
	cMutex.lock();
	if (CIT == nullptr) {
		debugGen << "Starting a color image collection thread...\n";
		ColorImageThread::initialize();
		CIT = new ColorImageThread();							//Create a ColorImageThread if it hasn't been created yet. This way, a thread is started only if it is required
	}
	else {
		debugGen << "Using existing color image collection thread...\n";
	}
	CITUsers++;
	addConnection(C, "Color_Images");
	cMutex.unlock();

	processClientMessages(CIT, C);

	cMutex.lock();
	CITUsers--;
	if (CITUsers == 0) {
		debugGen << "Last client disconnected from Color image server. Shutting down thread...\n";
		delete CIT;
		CIT = nullptr;
		ColorImageThread::finalize();
	}
	removeConnection(C);
	cMutex.unlock();
}

void InfraredImageServer(Client *C) {
	debugGen << C->getIP() << " successfully connected to Infrared image server\n";
	iMutex.lock();
	if (!IIT) {
		debugGen << "Starting a infrared image collection thread...\n";
		InfraredImageThread::initialize();
		IIT = new InfraredImageThread();						//Create a InfraredImageThread if it hasn't been created yet. This way, a thread is started only if it is required	
	}
	else {
		debugGen << "Using existing infrared image collection thread...\n";
	}
	IITUsers++;
	addConnection(C, "Infrared_Images");
	iMutex.unlock();

	processClientMessages(IIT, C);

	iMutex.lock();
	IITUsers--;
	removeConnection(C);
	if (IITUsers == 0) {
		debugGen << "Last client disconnected from Infrared image server. Shutting down thread...\n";
		delete IIT;
		IIT = nullptr;
		InfraredImageThread::finalize();
	}
	iMutex.unlock();
}

void DepthMapServer(Client *C) {
	debugGen << C->getIP() << " successfully connected to Depth map server\n";

	dMutex.lock();
	if (!DMT) {
		debugGen << "Starting a depth map collection thread...\n";
		DepthMapThread::initialize();
		DMT = new DepthMapThread();								//Create a DepthMapThread if it hasn't been created yet. This way, a thread is started only if it is required
	}
	else {
		debugGen << "Using existing depth map collection thread...\n";
	}
	DMTUsers++;
	addConnection(C, "Depth_Map");
	dMutex.unlock();

	processClientMessages(DMT, C);

	dMutex.lock();
	DMTUsers--;
	removeConnection(C);
	if (DMTUsers == 0) {
		debugGen << "Last client disconnected from depth map server. Shutting down thread...\n";
		delete DMT;
		DMT = nullptr;
		DepthMapThread::finalize();
	}
	dMutex.unlock();
}

void BodyMapServer(Client *C) {
	debugGen << C->getIP() << " successfully connected to Body map server\n";

	bMutex.lock();
	if (!BMT) {
		debugGen << "Starting a body map collection thread...\n";
		BodyMapThread::initialize();
		BMT = new BodyMapThread();								//Create a BodyMapThread if it hasn't been created yet. This way, a thread is started only if it is required
	}
	else {
		debugGen << "Using existing body map collection thread...\n";
	}
	BMTUsers++;
	addConnection(C, "Body_Map");
	bMutex.unlock();

	processClientMessages(BMT, C);

	bMutex.lock();
	BMTUsers--;
	removeConnection(C);
	if (BMTUsers == 0) {
		debugGen << "Last client disconnected from body map server. Shutting down thread...\n";
		delete BMT;
		BMT = nullptr;
		BodyMapThread::finalize();
	}
	bMutex.unlock();
}

void DebugServer(Client *C) {
	debugGen << C->getIP() << " successfully connected to debug server\n";

	addConnection(C, "Debugger");
	DebugUser dUser;

	while (true) {
		string msg = dUser.getNextMsg();
		if (msg != "") {
			int i = C->send(msg);
			if (i <= 0) break;
		}
	}
	removeConnection(C);
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
	debugGen << C->getIP() << " successfully connected to Body skeleton server\n";

	jMutex.lock();
	if (!BT) {
		debugGen << "Starting a body skeleton data collection thread...\n";
		BodyThread::initialize();
		BT = new BodyThread();
		//Sleep(1000);
	}
	else {
		debugGen << "Using existing body skeleton collection thread...\n";
	}
	BTUsers++;
	addConnection(C, "Skeleton_Data");
	jMutex.unlock();
	C->disableNagles();
	clock_t lastSendTime = clock();
	string s;
	C->receive(s);
	debugGen << "Skeleton Server: " << C->getIP() << " : " << s << "\n";
	vector<string> strs = split(s);
	if (strs[0] != "get") {
		debugGen << "Skeleton Server: " << C->getIP() << " : " << "Invalid command: " << strs[0] << ". Should be get. Disconnecting\n";
	}
	else {
		while (true) {
			while (clock() - lastSendTime < 16) {
				Sleep(clock() - lastSendTime);
			}
			OutputDebugString((to_string(clock() - lastSendTime) + "\n"s).c_str());
			lastSendTime = clock();
			int i = 1;
			vector<char> output;
			while (strs.size() > i)
			{
				JointType query;
				if (isalpha(strs[i + 1][0]))
					query = jointNames[strs[i + 1]];
				else
					query = (JointType)stoi(strs[i + 1]);

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
				int x, y, z;
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
			if (success < 0) {
				debugGen << "Skeleton Server: " << C->getIP() << " disconnected\n";
				break;
			}
			//OutputDebugString(outputStr);
		}
	}

	jMutex.lock();
	BTUsers--;
	removeConnection(C);
	if (BTUsers == 0) {
		debugGen << "Last client disconnected from body skeleton server. Shutting down thread...\n";
		delete BT;
		BT = nullptr;
		BodyThread::finalize();
	}
	jMutex.unlock();
}

#include <iostream>
void SpeechServer(Client *C) {
	if (ATUsers > 0) {
		debugGen << "Only one client can use audio at a time. Access denied to " << C->getIP() << "\n";
		return;
	}
	C->disableNagles();

	debugGen << C->getIP() << " successfully connected to Audio server\n";

	aMutex.lock();
	if (!AT) {
		debugGen << "Starting a audio collection thread...\n";
		AudioThread::initialize();
		AT = new AudioThread();
	}
	else {
		debugGen << "Using existing audio collection thread...\n";
		debugGen << "!!! WARNING !!! There should never be an existing thread. Only one user is allowed !!!";
	}
	ATUsers++;
	addConnection(C, "Speech_Recog");
	aMutex.unlock();

	int id = AT->registerUser();

	string inp;
	C->receive(inp, "\r\n");
	AT->setGrammar(id, split(inp), 1);

	//Set a time-out now.
	C->setTimeout(10);

	//AT->setGrammar(id,inp);
	cout << "Done" << endl;

	while (true) {
		string phrase;

		//Check for any requests from the client
		{
			string request;
			if (C->receive(request, "\n\r") > 0) {
				auto unpackedRequest = split(request);
				C->send(request);
				if (unpackedRequest[0] == "set") {
					AT->setGrammar(id, unpackedRequest, 1);
					cout << "Grammar changed" << endl;
				}
				else if (unpackedRequest[0] == "disconnect") {
					break;
				}
			}
		}

		if (AT->getSpokenPhrase(id, phrase)) {
			cout << "Sending " << phrase << endl;
			if (C->send(phrase) < 0) break;
			if (C->send("\n", 1) < 0) break;
		}
		//if (C->send("") < 0) break;
		//Sleep(5);		//No need of sleep as receive has a 10 ms time out anyway
	}

	AT->unregisterUser(id);
	aMutex.lock();
	ATUsers--;
	C->close();
	removeConnection(C);
	if (ATUsers == 0) {
		debugGen << "Last client disconnected from audio server. Shutting down thread...\n";
		delete AT;
		AT = nullptr;
		AudioThread::finalize();
	}
	else {
		debugGen << "!!! WARNING !!! More than one user on audio server !!!\n";
	}
	aMutex.unlock();
}